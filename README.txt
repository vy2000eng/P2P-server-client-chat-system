
client compilation:
gcc client/client.c client/main.c client/greeting.c shared/utils.c -o client/cli -pthread

server compilation:
gcc server/server.c server/main.c shared/utils.c -o server/ser -pthread


