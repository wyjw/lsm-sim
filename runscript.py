import os
import subprocess
import sys
import os.path
import logging
import multiprocessing

def run_command(command, policy, fstr, appid, hide_stderr = True, shell=True):
    if hide_stderr:
        kw = {'stderr' : subprocess.PIPE}
    else:
        kw = {}

    f = open("results/" + fstr + policy + "app" + str(appid) + ".r", "w")
    logging.info("Running `%s`", " ".join(list(map(str, command))))

    sys.stdout.flush()
    sys.stderr.flush()

    proc = subprocess.Popen(command, universal_newlines=True, shell=True, stdout=f, **kw)

    try:
        stderr = proc.communicate()[1]
    except:
        if proc.stderr:
            proc.stderr.close()
        try:
            proc.kill()
        except OSError:
            pass
        proc.wait()
        raise

    if proc.returncode != 0:
        if hide_stderr:
            sys.stderr.flush()
            sys.stderr.write(stderr)
            sys.stderr.flush()
        raise RuntimeError("Test died")

def get_command(cache_size, policy, appid):
    base_cmd = "./lsm-sim -f /media/onetbdrive/m.cap.out"
    final_cmd = base_cmd + " -s " + str(cache_size) + " -p " + str(policy) + " -a " + str(appid)
    print("Running " + final_cmd + '\n', flush=True)
    run_command(final_cmd, policy, str(cache_size), appid)

if __name__ == "__main__": 
    policies = ['lru', 'shadowlru', 'fifo']
    cache_sizes = [1000000000, 2000000000, 4000000000, 8000000000, 16000000000]
    processes = []
    for appid in range(1,20):
        for policy in policies:
            for cache_size in cache_sizes:
                p = multiprocessing.Process(target = get_command, args=(cache_size, policy, appid))
                processes.append(p)
                p.start()

            for process in processes:
                process.join()
