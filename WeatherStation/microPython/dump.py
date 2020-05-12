import btree
import sys

def dump():
    try:
        f = open("iotdb.db","r+b")
    except OSError:
        print("Failed to open db.")
        sys.exit(0)
    
    db = btree.open(f)
    
    for key in db:
        print("Key  : ",key )
        print("Value: ", db[key])
    
