import uasyncio as asyncio
async def bar():
    count = 0
    while True:
        count += 1
        print(count)
        await asyncio.sleep(1)  # Pause 1s

loop = asyncio.get_event_loop()
loop.create_task(bar()) # Schedule ASAP
loop.run_forever()
