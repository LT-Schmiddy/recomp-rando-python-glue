import asyncio, threading
from collections.abc import Awaitable
from concurrent.futures import Future

class AsyncLoopThread:
    _running: bool
    _thread: threading.Thread
    
    # Safe to be used by the background thread:
    _loop_created_event: threading.Event
    _exit_event: threading.Event
    _event_loop: asyncio.AbstractEventLoop
    _quantum: float
    
    def __init__(self, quantum: float = 0.01):
        # Initialize all values:
        self._running = False
        self._thread = threading.Thread(target=self._thread_func)
        self._event_loop = None
        self._loop_created_event = threading.Event()
        self._exit_event = threading.Event()
        self._quantum = quantum
        
    def start(self):
        # Start the background thread.
        if not self._running:
            self._running = True
            self._thread.start()
            # We can't return before the loop is created. Otherwise, enqueue may not work.
            self._loop_created_event.wait()
        else:
            raise RuntimeError("AsyncLoopThread is already running.")
            
    def stop(self):
        if not self._running:
            raise RuntimeError("AsyncLoopThread is not running.")
        
        else:
            self._exit_event.set()
            self._thread.join()
            self._event_loop.close()
            self._running = False
    
    
    def enqueue(self, coro: Awaitable) -> Future:
        if not self._running:
            raise RuntimeError("AsyncLoopThread is not running.")
        
        return asyncio.run_coroutine_threadsafe(coro, self._event_loop)
    
    def _thread_func(self):
        self._event_loop = asyncio.new_event_loop()
        asyncio.set_event_loop(self._event_loop)
        
        self._loop_created_event.set()
        
        # This will allow us to check for the exit event:
        while not self._exit_event.is_set():
            # Run the loop for a limited period of time.
            self._event_loop.call_later(self._quantum, self._event_loop.stop)
            self._event_loop.run_forever()
            
        self._event_loop.run_until_complete(self._shutdown())
        
    async def _shutdown(self):
        tasks = asyncio.all_tasks(self._event_loop)
        if len(tasks) == 0:
            return
        
        for task in tasks:
            task.cancel()
            
        await asyncio.gather(*tasks, return_exceptions=True)