import asyncio
import aiohttp
import time
import random

async def fetch(session, url, success_count, failure_count, latency):
    try:
        async with session.get(url) as response:
            await asyncio.sleep(latency)  # Simulate network latency
            if response.status == 200:
                success_count.append(1)
            else:
                failure_count.append(1)
            return await response.text()
    except Exception as e:
        failure_count.append(1)
        return None

async def main():
    url = 'http://localhost:8080'  # replace with your server URL
    tasks = []
    success_count = []
    failure_count = []
    num_calls = 100000
    avg_latency_ms = .050  # Average latency in seconds

    with open("results.txt", 'w') as results:
        for i in range(10):
            async with aiohttp.ClientSession() as session:
                for _ in range(num_calls):
                    # Simulate variable latency around the average
                    task = asyncio.create_task(fetch(session, url, success_count, failure_count, avg_latency_ms))
                    tasks.append(task)
                
                start_time = time.time()
                await asyncio.gather(*tasks)
                end_time = time.time()

                results.write(f"Attempt {i}: Time taken for {num_calls} concurrent requests: {end_time - start_time} seconds\n")

    print(f"Time taken for {num_calls * 10} concurrent requests: {end_time - start_time} seconds")
    print(f"Number of successful requests: {len(success_count)}")
    print(f"Number of failed requests: {len(failure_count)}")

if __name__ == '__main__':
    asyncio.run(main())



# import asyncio
# import aiohttp
# import time

# async def fetch(session, url):
#     async with session.get(url) as response:
#         return await response.text()

# async def main():
#     url = 'http://localhost:8080'  # replace with your server URL
#     tasks = []

#     async with aiohttp.ClientSession() as session:
#         for _ in range(1000):
#             task = asyncio.create_task(fetch(session, url))
#             tasks.append(task)
        
#         start_time = time.time()
#         await asyncio.gather(*tasks)
#         end_time = time.time()

#     print(f"Time taken for 1000 concurrent requests: {end_time - start_time} seconds")

# if __name__ == '__main__':
#     asyncio.run(main())
