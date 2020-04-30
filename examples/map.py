import itertools
from typing import *  # use type hints to make signatures clear

import snakefish

calc_time = {}  # global variable "calc_time"


# the function that will extract shared global variables
def extract(globals_dict: Dict[str, Any]) -> Dict[str, Any]:
    return {"calc_time": globals_dict["calc_time"]}  # extract calc_time


# the function that will merge shared global variables
def merge(old_globals: Dict[str, Any], new_globals: Dict[str, Any]) -> None:
    old_globals["calc_time"].update(new_globals["calc_time"])


def f1(i: int) -> int:
    calc_time[i] = snakefish.get_timestamp()
    return i ** 2


def f2(i: int, j: int) -> int:
    return i * j


print("calc_time was %s\n" % calc_time)

args = range(100)
results = snakefish.map(f1, args, extract, merge, concurrency=4, chunksize=7)  # with merging
print("map results are %s\n" % results)
assert (results == list(map(f1, args)))

print("calc_time is %s" % calc_time)

args = [(i, i + 1) for i in range(50)]
results = snakefish.starmap(f2, args)  # no merging, default concurrency & chunksize
print("starmap results are %s\n" % results)
assert (results == list(itertools.starmap(f2, args)))
