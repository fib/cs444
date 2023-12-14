**Producer:**
- Loop:
    1. acquire queue mutex
    2. generate customer
    3. signal if queue length went 0 -> 1
    4. release queue mutex

**Consumer:**
- Loop:
    1. acquire queue mutex
    2. while queue is empty, wait for signal
    3. serve customer
    4. release queue mutex