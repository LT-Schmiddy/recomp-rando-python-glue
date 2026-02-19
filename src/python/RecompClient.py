import asyncio
import time

import Utils
import websockets
import functools
from copy import deepcopy
from typing import List, Any, Iterable
from NetUtils import decode, encode, JSONtoTextParser, JSONMessagePart, NetworkItem, NetworkPlayer, ClientStatus
from MultiServer import Endpoint
from CommonClient import CommonContext, ClientCommandProcessor, logger

class RecompContext(CommonContext):
	game = "Recomp Rando Glue" # TODO: allow for this to be changed (used for auth/knowing what game you're reading from?)

	def __init__(self, server_address, password):
		super().__init__(server_address, password)
		self.autoreconnect_task = None # is this redundant?
		self.items_handling = 0b111 # allow for all items to come through/be processed

		# taken from AHIT client
		self.game_connected = False
		self.awaiting_info = False
		self.full_inventory: List[Any] = []

		# our own variables
		self.deathlink_enabled = False
		self.deathlink_pending = False
		
	# TODO: actually handle this lol
	async def server_auth(self, password_requested: bool = False):
		if password_requested and not self.password:
			pass # TODO: broadcast error message instead

		# await self.get_username()
		await self.send_connect()

	def is_connected(self) -> bool:
		return self.server and self.server.socket.open
	
	# TODO: custom handling per package here
	# def on_package(self, cmd: str, args: dict):
	# 	pass

	# TODO: move ALL of this into individual recomp functions

	# location info
	def rando_location_is_checked(self, location_id: int):
		return location_id in self.locations_checked
		# return location_id in self.checked_locations # checked_locations is serverside checked while locations_checked in client
	
	def rando_get_location_type(self, location_id: int):
		return self.locations_info[location_id].flags
	
	def rando_get_location_has_local_item(self, location_id: int):
		return self.locations_info[location_id].player == self.slot
	
	def rando_get_item_at_location(self, location_id: int):
		return self.locations_info[location_id].item
	
	def rando_location_exists(self, location_id: int):
		return location_id in self.server_locations

	async def rando_send_location(self, location_id: int):
		# would it be better to add locations to a list that are sent out in the update loop?
		self.check_locations([int]) # typing.Collection[int]
	
	# item info
	def rando_has_item(self, item_id: int):
		for item in self.items_received:
			if item_id == item.item:
				return True
		return False
	
	def rando_get_item_location(self, item_id: int): # ???
		for item in self.items_received:
			if item_id == item.item:
				return item.location
		return 0
	
	# deathlink info
	def rando_get_death_link_pending(self):
		return self.deathlink_pending
	
	def rando_reset_death_link_pending(self):
		self.deathlink_pending = False
	
	def rando_get_death_link_enabled(self):
		return "DeathLink" in self.tags
	
	def rando_update_deathlink_state(self, state: bool): # new function
		self.update_death_link(state)
	
	def rando_send_death_link(self):
		self.send_death() # add option for custom text?
		self.rando_reset_death_link_pending()

	def on_deathlink(self, data: typing.Dict[str, typing.Any]) -> None:
		self.deathlink_pending = True
		super().on_deathlink # TODO: handle this more thoughroughly (custom deathlink messages?)

	# Goal
	async def rando_complete_goal(self):
		self.finished_game = True
		await self.send_msgs([{"cmd": "StatusUpdate", "status": ClientStatus.CLIENT_GOAL}]) # probably