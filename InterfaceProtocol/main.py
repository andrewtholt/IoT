
import espSide

def main():
    conn = espSide.espSide()

    print("e.g. cut and paste")

    print('{ "CMD": { "NAME":"WIFI_CONNECT", "ESSID":"HoltAtHome4","PASSWD":"password"}}')


    run = True

    while run:
        tst = input()
        print(conn.command(tst))

        run = conn.queryCommandLoop()

    print("Command Loop",conn.queryCommandLoop())

main()

