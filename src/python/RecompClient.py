import asyncio
import time

import Utils
import websockets
import functools
import typing
# from copy import deepcopy
# from typing import List, Any, Iterable
from NetUtils import decode, encode, JSONtoTextParser, JSONMessagePart, NetworkItem, NetworkPlayer, ClientStatus
from MultiServer import Endpoint
from CommonClient import CommonContext, ClientCommandProcessor, logger, server_loop, get_base_parser, handle_url_arg

import logging
import copy

import recomp_data

from rando_async_controller import AsyncLoopThread

class RecompContext(CommonContext):
    def __init__(self, server_address, password):
        super().__init__(server_address, password)
        self.autoreconnect_task = None # is this redundant?
        self.items_handling = 0b111 # allow for all items to come through/be processed

        # taken from AHIT client
        self.game_connected = False
        self.awaiting_info = False
        self.full_inventory: List[Any] = []

        # our own variables
        self.slot_data = dict()
        self.deathlink_enabled = False
        self.deathlink_pending = False
        
    # TODO: actually handle this lol
    async def server_auth(self, password_requested: bool = False):
        if password_requested and not self.password:
            pass # TODO: broadcast error message instead

        await self.get_username()
        await self.send_connect()

    def is_connected(self) -> bool:
        return self.server and self.server.socket.open
    
    # custom package handling
    def on_package(self, cmd: str, args: dict):
        if cmd == 'Connected':
            self.slot_data = args.get("slot_data", {})

class TextContext(CommonContext):
    # Text Mode to use !hint and such with games that have no text entry
    tags = CommonContext.tags | {"TextOnly"}
    game = ""  # empty matches any game since 0.3.2
    items_handling = 0b111  # receive all items for /received
    want_slot_data = False  # Can't use game specific slot_data

    async def server_auth(self, password_requested: bool = False):
        if password_requested and not self.password:
            await super(TextContext, self).server_auth(password_requested)
        await self.get_username()
        await self.send_connect(game="")

    def on_package(self, cmd: str, args: dict):
        if cmd == "Connected":
            self.game = self.slot_info[self.slot].game

    async def disconnect(self, allow_autoreconnect: bool = False):
        self.game = ""
        await super().disconnect(allow_autoreconnect)

async def async_main(args):
    # ctx = TextContext(args.connect, args.password)
    ctx = recomp_data.ctx
    ctx.server_task = asyncio.create_task(server_loop(ctx), name="server loop")

    ctx.run_cli() # force cli output

    await ctx.exit_event.wait()
    await ctx.shutdown()

# temp, easier to modify if needed
def run_as_textclient(*args):    
    import colorama

    parser = get_base_parser(description="Gameless Archipelago Client, for text interfacing.")
    parser.add_argument('--name', default=None, help="Slot Name to connect as.")
    parser.add_argument("url", nargs="?", help="Archipelago connection url")
    args = parser.parse_args(args)

    args = handle_url_arg(args, parser=parser)

    # use colorama to display colored text highlighting on windows
    colorama.just_fix_windows_console()

    async_thread_loop = AsyncLoopThread()
    async_thread_loop.start()
    async_thread_loop.enqueue(async_main(args))
    return async_thread_loop
    # colorama.deinit()

async def setup_ctx(game):
    ctx = RecompContext('', '')
    ctx.game = game
    recomp_data.ctx = ctx

def init_ctx(game):
    async_thread_loop = AsyncLoopThread()
    async_thread_loop.start()
    async_thread_loop.enqueue(setup_ctx(game)).result()