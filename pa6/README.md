# PA 6

## Run instruction

1. build docker image
    ```bash
    docker build -t pa6 .
    ```
2. run docker-container in interactive mode
    ```bash
    docker run -it --name pa6-container pa6
    ```
3. inside container
    ```bash
    clang -std=c99 -Wall -pedantic -L. -lruntime *.c -o main
    ```

    ```bash
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/pa6/
    ```

    ```bash
    LD_PRELOAD="${PWD}/libruntime.so" ./main -p 3 [--mutexl]
    ```