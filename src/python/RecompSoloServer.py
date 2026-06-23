import Utils

from MultiServer import Context, server, console, auto_shutdown, server_per_message_deflate_factory, parse_args
# from MultiServer import main as server_main

from pathlib import Path
from signal import SIGINT, SIGTERM, signal
import argparse
import logging
import websockets
import functools
import asyncio
import typing
import time

import recomp_data
from rando_async_controller import AsyncLoopThread

async_thread = AsyncLoopThread()
async_thread.start()

# unsure if this will get used
# class RecompServerContext(Context):
# 	def __init__(self, host: str, port: int, server_password: str, password: str, location_check_points: int,
# 				 hint_cost: int, item_cheat: bool, release_mode: str = "disabled", collect_mode="disabled",
# 				 countdown_mode: str = "auto", remaining_mode: str = "disabled", auto_shutdown: typing.SupportsFloat = 0, 
# 				 compatibility: int = 2, log_network: bool = False, logger: logging.Logger = logging.getLogger()):
# 		super().__init__(host, port, server_password, password, location_check_points, hint_cost, item_cheat, release_mode, collect_mode,
# 							countdown_mode, remaining_mode, auto_shutdown, compatibility, log_network, logger)

# this whole function was copied mainly to save the ctx object in recomp_data + other small changes
async def server_main(args: argparse.Namespace):
	Utils.init_logging(name="Server",
					   loglevel=args.loglevel.lower(),
					   add_timestamp=args.logtime)

	recomp_data.server_ctx = Context(args.host, args.port, args.server_password, args.password, args.location_check_points,
									args.hint_cost, not args.disable_item_cheat, args.release_mode, args.collect_mode,
									args.countdown_mode, args.remaining_mode,
									args.auto_shutdown, args.compatibility, args.log_network)
	ctx = recomp_data.server_ctx
	data_filename = args.multidata

	if not data_filename: # shouldn't happen with intended use use
		# TODO: show error message in notifications
		recomp_data.error_msg = "No file selected"
		print(recomp_data.error_msg)
		raise # does raise cause issues?

	try:
		ctx.load(data_filename, args.use_embedded_options)

	except Exception as e:
		logging.exception()
		recomp_data.error_msg = f"Failed to read multiworld data ({e})"
		print(recomp_data.error_msg)
		raise

	ctx.init_save(not args.disable_save)

	# ssl_context = load_server_cert(args.cert, args.cert_key) if args.cert else None
	ssl_context = None

	ctx.server = websockets.serve(
		functools.partial(server, ctx=ctx),
		host=ctx.host,
		port=ctx.port,
		ssl=ssl_context,
		extensions=[server_per_message_deflate_factory],
	)
	# ip = args.host if args.host else Utils.get_public_ipv4()
	ip = args.host if args.host else "unknown" # should always be localhost unless something went wrong
	logging.info('Hosting game at %s:%d (%s)' % (ip, ctx.port,
												 'No password' if not ctx.password else 'Password: %s' % ctx.password))

	await ctx.server
	console_task = asyncio.create_task(console(ctx))
	if ctx.auto_shutdown:
		ctx.shutdown_task = asyncio.create_task(auto_shutdown(ctx, [console_task]))

	def stop():
		try:
			for remove_signal in [SIGINT, SIGTERM]:
				asyncio.get_event_loop().remove_signal_handler(remove_signal)
		except NotImplementedError:
			pass
		ctx.commandprocessor._cmd_exit()

	def shutdown(signum, frame):
		stop()

	# probably don't need this? (assuming its ctrl + c shutdown)
	try:
		for sig in [SIGINT, SIGTERM]:
			asyncio.get_event_loop().add_signal_handler(sig, stop)
	except NotImplementedError:
		# add_signal_handler is only implemented for UNIX platforms
		for sig in [SIGINT, SIGTERM]:
			signal(sig, shutdown)

	await ctx.exit_event.wait()
	console_task.cancel()
	if ctx.shutdown_task:
		await ctx.shutdown_task

# totally not taken directly from stack overflow :P
def is_port_in_use(port: int) -> bool:
    import socket
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        return s.connect_ex(('localhost', port)) == 0

def wait_for_server(timeout, period):
	timeout_time = time.time() + timeout
	# start_time = time.time()

	while time.time() < timeout_time:
		if recomp_data.server_ctx and is_port_in_use(recomp_data.server_ctx.port):
			# print("server open", time.time() - start_time)
			return True
		# print("server not available", time.time() - start_time)
		
		time.sleep(period)
		
	recomp_data.error_msg = "Solo Context timed out."
	return False

def start_server(multidata = "", port = 38281):
	global async_thread
	
	args = parse_args()

	if multidata:
		args.multidata = multidata
	else:
		# grab latest generated seed for testing
		output_folder = recomp_data.mod_data_path.joinpath("Archipelago", "local", "output")
		output_files = output_folder.glob("*.zip")
		latest_file = max(output_files, key=lambda item: item.stat().st_ctime)
		args.multidata = str(latest_file)
		print(str(latest_file))

	args.host = "localhost"
	args.port = port
	async_thread.enqueue(server_main(args))
	
	# the server isn't immediately usable once we try to open it, so we have to sleep until it is
	# from my own testing, basically any small period works (only 0.1 gave a small enough interval to be faster though?)
	# shouldn't take long at all, but just in case there's a delay between the server starting and the connection or a failure
	server_open = wait_for_server(5, 0.1)
	if not server_open: # just in case the server's taking too long to start up
		recomp_data.server_ctx.commandprocessor._cmd_exit() # probably shuts down the server if it ended up starting?
	return server_open