import asyncio
import time
import typing
from pathlib import Path

import Utils
import websockets
import functools
from NetUtils import decode, encode, JSONtoTextParser, JSONMessagePart, NetworkItem, NetworkPlayer, ClientStatus
from CommonClient import CommonContext, server_loop

import recomp_data
from rando_async_controller import AsyncLoopThread

async_thread = AsyncLoopThread()
async_thread.start();

class RecompContext(CommonContext):
    def __init__(self, server_address, password):
        super().__init__(server_address, password)
        self.items_handling = 0b111 # allow for all items to come through/be processed

        self.recieved_item_ids: List[Any] = [] # mirrors items_received, but is only the item ids

        self.connection_success = False
        self.connection_failed = False
        self.failed_reason = ""

        self.slot_data = dict()
        self.deathlink_enabled = False
        self.deathlink_pending = False
        self.recomp_needs_updating = False
        self.local_checked = set()
        # TODO: set self.locations_checked from file saving self.local_checked
        
    # TODO: actually handle this lol
    async def server_auth(self, password_requested: bool = False):
        if password_requested and not self.password:
            pass # TODO: broadcast error message instead

        await self.get_username()
        await self.send_connect()

    # lets the game know why the connection failed (no reconnect)
    def handle_connection_loss(self, msg: str) -> None:
        self.connection_failed = True
        self.failed_reason = msg
        super().handle_connection_loss(str)

    def is_connected(self) -> bool:
        return self.server and self.server.socket.open
    
    def on_deathlink(self, data: typing.Dict[str, typing.Any]) -> None:
        self.deathlink_pending = True
        super().on_deathlink(data)
    
    async def complete_goal(self) -> None:
        await self.send_msgs([{"cmd": "StatusUpdate", "status": ClientStatus.CLIENT_GOAL}])

    # override `check_locations` to save sent locations (make super later)
    async def check_locations(self, locations: typing.Collection[int]) -> set[int]:
        """Send new location checks to the server. Returns the set of actually new locations that were sent."""
        self.recomp_needs_updating = True
        self.local_checked |= set(locations)
        self.locations_checked |= set(locations) # just in case we need to resend these after a disconnect(?)
        
        locations = set(locations) & self.missing_locations
        if locations:
            await self.send_msgs([{"cmd": 'LocationChecks', "locations": tuple(locations)}])
        return locations

    # custom package handling
    def on_package(self, cmd: str, args: dict):
        if cmd == 'Connected':
            self.connection_success = True
            self.slot_data = args.get("slot_data", {})
            self.recomp_needs_updating = True
            self.locations_checked |= self.checked_locations # just in case(?)
            self.local_checked |= self.checked_locations
        elif cmd == "RoomInfo":
            self.seed_name = args["seed_name"]
        elif cmd == "RoomUpdate":
            # maybe this could be used to show notifications for items collected by other players on the same slot
            if "checked_locations" in args:
                self.recomp_needs_updating = True
                self.local_checked |= self.checked_locations
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
    global async_thread
    import colorama

    # use colorama to display colored text highlighting on windows
    colorama.just_fix_windows_console()

    async_thread.enqueue(async_main())
    return async_thread
    # colorama.deinit()

async def setup_ctx(game):
    ctx = RecompContext('', '')
    ctx.game = game
    recomp_data.ctx = ctx

def run_async_task_once(async_func):
    global async_thread
    async_thread.enqueue(async_func)

def run_async_task_and_wait_once(async_func):
    global async_thread
    async_thread.enqueue(async_func).result()

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

# TODO: clean up whole file later
def wait_for_connection(timeout, period):
    timeout_time = time.time() + timeout
    ctx = recomp_data.ctx
    ctx.connection_failed = False

    while time.time() < timeout_time:
        if ctx.connection_success:
            return True
        
        elif ctx.connection_failed:
            return False
        
        time.sleep(period)
        
    ctx.connection_failed = True
    ctx.failed_reason = "Connection timed out."
    return False