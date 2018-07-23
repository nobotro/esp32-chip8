from pynput.keyboard import Key, Listener
import socket
import time

#UDP_IP = "127.0.0.1"
UDP_IP = "192.168.4.1"
UDP_PORT = 1327
PREV_KEY=None
import sys
game=None

def send_command(command):
    print(command)
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(str(command).encode(), (UDP_IP, UDP_PORT))

def main(argv):
    global game


    print('''
///////


    game:brix
    players:1
    controls:
        p1:left-a,right-d
        need reset after end(press r)
        
    game:filter
    players:1
    controls:
        p1:left-a,right-d
        need reset after end(press r)
        
       
    game:pongai
    players:1
    controls:
        p1:up-w,down-s
        

 
    game:pong2
    players:2
    controls:
        p1:up-w,down-s
        p2:up-key.up,key.down
     
   
    

    
        
    game:spaceinvaders(not work good)
    players:1
    controls:
        p1:left-a,right-d,shot-w,start-s
        
    game:tetris((not work good)5wawwaw6656r666e6rr
    players:1
    controls:
        p1:left-a,right-d,flip-w,fast-s
        need reset after end(press r)''')

    with Listener(
            on_press=on_press,
            on_release=on_release) as listener:
        listener.join()





def on_press(key):
    global PREV_KEY
    global game
    key = str(key)

    if key == "'1'":
        game='brix'
        if key == PREV_KEY:
            return
        else:
            PREV_KEY = key
        send_command('k>1')
        return
    if key == "'2'":
        game = 'filter'
        if key == PREV_KEY:
            return
        else:
            PREV_KEY = key
        send_command('k>2')
        return
    if key == "'3'":
        game = 'pongai'
        if key == PREV_KEY:
            return
        else:
            PREV_KEY = key
        send_command('k>3')
        return

    if key == "'4'":
        game='pong2'
        if key == PREV_KEY:
            return
        else:
            PREV_KEY = key
        send_command('k>4')
        return

    if key == "'5'":
        game='spaceinvaders'
        if key == PREV_KEY:
            return
        else:
            PREV_KEY = key
        send_command('k>5')
        return
    if key == "'6'":
        game='tetris'
        if key == PREV_KEY:
            return
        else:
            PREV_KEY = key
        send_command('k>6')
        return

    if key == "'w'" and (game=='pong2' or game=='pongai' or game =='spaceinvaders' or game=='tetris'):
        if key == PREV_KEY:
            return
        else:
            PREV_KEY = key

        send_command('k>w')
        return
    if key == "'s'" and (game=='pong2' or game=='pongai' or game =='spaceinvaders' or game=='tetris'):
        if key == PREV_KEY:
            return
        else:
            PREV_KEY = key

        send_command('k>s')
        return

    if key == "'d'" and (game=='filter' or game=='brix' or game =='spaceinvaders' or game=='tetris'):
        if key == PREV_KEY:
            return
        else:
            PREV_KEY = key
        send_command('k>d')
        return
    if key == "'a'" and (game=='filter' or game=='brix' or game =='spaceinvaders' or game=='tetris'):

        if key == PREV_KEY:
            return
        else:
            PREV_KEY = key
        send_command('k>a')
        return

    if key == "'r'":
        if key == PREV_KEY:
            return
        else:
            PREV_KEY = key
        send_command('k>r')
        return
    if key == "Key.up" and (game=='pong2'):
        if key == PREV_KEY:
            return
        else:
            PREV_KEY = key
        send_command('k>v')

    if key == "Key.down" and (game=='pong2'):
        if key == PREV_KEY:
            return
        else:
            PREV_KEY = key
        send_command('k>m')
        return

def on_release(key):

    global PREV_KEY
    global game


    key = '!' + str(key)
    if key == "!'w'" and (game=='pong2' or game=='pongai' or game =='spaceinvaders' or game=='tetris'):
        if key == PREV_KEY:
            return
        else:
            PREV_KEY = key
        send_command('k<w')
        return
    if key == "!'s'" and (game=='pong2' or game=='pongai' or game =='spaceinvaders' or game=='tetris'):
        if key == PREV_KEY:
            return
        else:
            PREV_KEY = key
        send_command('k<s')
        return
    if key == "!'d'" and (game=='filter' or game=='brix' or game =='spaceinvaders' or game=='tetris'):
        if key == PREV_KEY:
            return
        else:
            PREV_KEY = key
        send_command('k<d')
        return
    if key == "!'a'" and (game=='filter' or game=='brix' or game =='spaceinvaders' or game=='tetris'):
        if key == PREV_KEY:
            return
        else:
            PREV_KEY = key
        send_command('k<a')
        return
    if key == "!'r'":

            PREV_KEY = key
            return

    if key == "!Key.up" and (game == 'pong2'):
        if key == PREV_KEY:
            return
        else:
            PREV_KEY = key
        send_command('k<v')

    if key == "!Key.down" and (game == 'pong2'):
        if key == PREV_KEY:
            return
        else:
            PREV_KEY = key
        send_command('k<m')
        return


 



if __name__ == "__main__":
    main(sys.argv)
