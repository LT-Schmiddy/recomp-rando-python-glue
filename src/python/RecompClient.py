import asyncio

import Utils
import websockets
import functools
import typing
from pathlib import Path
from NetUtils import decode, encode, JSONtoTextParser, JSONMessagePart, NetworkItem, NetworkPlayer, ClientStatus
from CommonClient import CommonContext, server_loop

import recomp_data

from rando_async_controller import AsyncLoopThread

class RecompContext(CommonContext):
    def __init__(self, server_address, password):
        super().__init__(server_address, password)
        self.autoreconnect_task = None # is this redundant?
        self.items_handling = 0b111 # allow for all items to come through/be processed

        self.recieved_item_ids: List[Any] = [] # mirrors items_received, but is only the item ids

        self.slot_data = dict()
        self.deathlink_enabled = False
        self.deathlink_pending = False
        self.recomp_needs_updating = False
        self.last_known_checked = set()
        
    # TODO: actually handle this lol
    async def server_auth(self, password_requested: bool = False):
        if password_requested and not self.password:
            pass # TODO: broadcast error message instead

        await self.get_username()
        await self.send_connect()

    def is_connected(self) -> bool:
        return self.server and self.server.socket.open
    
    def on_deathlink(self, data: typing.Dict[str, typing.Any]) -> None:
        self.deathlink_pending = True
        super().on_deathlink(data)

    # custom package handling
    def on_package(self, cmd: str, args: dict):
        if cmd == 'Connected':
            self.slot_data = args.get("slot_data", {})
            self.recomp_needs_updating = True
        elif cmd == "RoomInfo":
            self.seed_name = args["seed_name"]
        elif cmd == "RoomUpdate":
            if "checked_locations" in args:
                self.recomp_needs_updating = True
        elif cmd == 'ReceivedItems':
            # probably dumb to reset the list every time
            self.recieved_item_ids = []
            for item in self.items_received:
                self.recieved_item_ids.append(item.item)

# client context should be set up before this is called
async def async_main():
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
    async_thread_loop.enqueue(async_func)

def run_async_task_and_wait_once(async_func):
    async_thread_loop = AsyncLoopThread()
    async_thread_loop.start()
    async_thread_loop.enqueue(async_func).result()

# yes this still saves as "apconnect.txt" for the bit, even though a json would be better
def save_ap_connect(address, player_name, password):
    ap_connect = Path(recomp_data.mod_data_path, "apconnect.txt")
    ap_connect.write_text(f"{address}\n{player_name}\n{password}")

def get_ap_connect():
    ap_connect = Path(recomp_data.mod_data_path, "apconnect.txt")
    
    if not ap_connect.exists(): # first time running the randomizer
        ap_connect.write_text("archipelago.gg:38281\nPlayer\n")

    connection_info = ap_connect.read_text().splitlines()
    
    if len(connection_info) < 3:
        connection_info.append("") # empty password
    
    return connection_info