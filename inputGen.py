import random

# generate 500 random 40 bit numbers
for i in range(500):
    print(random.randrange(1099511627775))

# generate 524 random operations
operations = [
    "LOAD MQ",
    "LOAD MQ,M(X)",
    "STOR M(X)",
    "LOAD M(X)",
    "LOAD -M(X)",
    "LOAD |M(X)|",
    "LOAD -|M(X)|",
    "JUMP M(X,0:19)",
    "JUMP M(X,20:39)",
    "JUMP +M(X,0:19)",
    "JUMP +M(X,20:39)",
    "ADD M(X)",
    "ADD |M(X)|",
    "SUB M(X)",
    "SUB |M(X)|",
    "MUL M(X)",
    "DIV M(X)",
    "LSH",
    "RSH",
    "STOR M(X,8:19)",
    "STOR M(X,28:39)",
]

for i in range(524):
    op = random.choice(operations)
    op = op.replace("X", str(random.randrange(1023)))
    print(op)
