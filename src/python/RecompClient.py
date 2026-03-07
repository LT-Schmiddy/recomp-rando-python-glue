import asyncio
import time

import Utils
import websockets
import functools
from copy import deepcopy
from typing import List, Any, Iterable
from NetUtils import decode, encode, JSONtoTextParser, JSONMessagePart, NetworkItem, NetworkPlayer, ClientStatus
from MultiServer import Endpoint
from CommonClient import CommonContext, ClientCommandProcessor, logger, server_loop, get_base_parser, handle_url_arg

from rando_async_controller import AsyncLoopThread



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
    ctx = TextContext(args.connect, args.password)
    ctx.auth = args.name
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