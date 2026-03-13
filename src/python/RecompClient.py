import asyncio

import Utils
import websockets
import functools
import typing
from NetUtils import decode, encode, JSONtoTextParser, JSONMessagePart, NetworkItem, NetworkPlayer, ClientStatus
from CommonClient import CommonContext, server_loop

import recomp_data

from rando_async_controller import AsyncLoopThread

class RecompContext(CommonContext):
    def __init__(self, server_address, password):
        super().__init__(server_address, password)
        self.autoreconnect_task = None # is this redundant?
        self.items_handling = 0b111 # allow for all items to come through/be processed

        self.full_inventory: List[Any] = []

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

async def async_main():
    # client context should be set up before this is called
    ctx = recomp_data.ctx
    ctx.server_task = asyncio.create_task(server_loop(ctx), name="server loop")

    ctx.run_cli() # force cli output

    await ctx.exit_event.wait()
    await ctx.shutdown()

def connect_client(*args):    
    import colorama

    # use colorama to display colored text highlighting on windows
    colorama.just_fix_windows_console()

    async_thread_loop = AsyncLoopThread()
    async_thread_loop.start()
    async_thread_loop.enqueue(async_main())
    return async_thread_loop
    # colorama.deinit()

async def setup_ctx(game):
    ctx = RecompContext('', '')
    ctx.game = game
    recomp_data.ctx = ctx

def run_async_task_once(async_func):
    async_thread_loop = AsyncLoopThread()
    async_thread_loop.start()
    async_thread_loop.enqueue(async_func).result()