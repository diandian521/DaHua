import multiprocessing as MP
import time as T
def job(q,m):
    result = 0
    for i in range(m):
        T.sleep(0.05)
        result += i + i**2 + i**3
    q.put(result)
    
if __name__ == '__main__':
    startTime = T.time()
    q = MP.Queue()
    pro1 = MP.Process(target = job, args = (q,100))
    pro2 = MP.Process(target = job, args = (q,200))
    pro1.start()
    pro2.start()
    pro1.join()
    pro2.join()
    result1 = q.get()
    result2 = q.get()
    endTime = T.time()
    
    print('resut1 is： ', result1)
    print('resut2 is： ', result2)
    print('time use： ', endTime - startTime)
