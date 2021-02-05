#!/bin/bash
clear

function bail() {
  echo -e "\e[0;31mBailing for Reasons:"
  echo "$1"
  echo -ne "\e[0m"
  exit 1
}

                                         
                                         
echo -e "\e[0;33m    ___  ___ _ __  _ __ ___  ___ ___  ___  "
echo "   / _ \/ __| '_ \| '__/ _ \/ __/ __|/ _ \ "
echo "  |  __/\__ \ |_) | | |  __/\__ \__ \ (_)  "
echo "   \___||___/ .__/|_|  \___||___/___/\___/ "
echo "            | |                            "
echo "            | | github.com/donny-nyc       "
echo -ne "\e[0m"
echo

echo -ne "[ ] Build the server binary\\r"
make server > build.log 2>&1

if [ $? -eq 1 ]
then
  echo -e "\e[0;31m[!] Build the server binary"
  echo "    !! unble to build the binary !!"
  cat build.log
  echo -ne "\e[0m"
  exit 1
fi
echo -e "\e[0;32m[X] Build the server binary"
echo -ne "\e[0m"

echo -ne "\e[0;37m[ ] stop the espresso service\\r"
STOP_SERVICE_STATUS=$(ssh -i ~/.ssh/id_donny_nyc root@104.131.30.208 -C "systemctl stop espresso" 2>&1)
if [ ! $? -eq 0 ]
then
	echo -e "\e[0;31m[!] stop the espresso service\\r"
	bail "$STOP_SERVICE_STATUS"
fi
echo -e "\e[0;32m[X] stop the espresso service\\r"
echo -ne "\e[0m"

echo -ne "\e[0;37m[ ] Upload the binary to benchmark.nyc\\r"
if [ ! -f build/espresso ]
then
  echo -e "\e[0;31m[!] Upload the binary to benchmark.nyc"
  echo "     !! Binary not Found !!"
  echo -ne "\e[0m"
  exit 1
fi
UPLOAD_STATUS=$(scp -i ~/.ssh/id_donny_nyc build/espresso root@104.131.30.208:/usr/local/bin/espresso 2>&1)

if [ $? -eq 1 ] && [ "$1" != "static" ]
then
  echo -e "\e[0;31m[!] Upload the binary to benchmark.nyc"
  echo "    !! Could not upload binary !!"
  echo -ne "\e[0m"
	bail "$UPLOAD_STATUS"
fi
echo -e "\e[0;32m[X] Upload the binary to benchmark.nyc"

echo -ne "\e[0m"


echo -ne "\e[0;37m[ ] Upload static files\\r"
STATIC_STATUS=$(scp -i ~/.ssh/id_donny_nyc public/index.html root@104.131.30.208:/home/espresso/public/index.html 2>&1; scp -i ~/.ssh/id_donny_nyc static/favicon.png root@104.131.30.208:/home/espresso/static/favicon.png 2>&1)
if [ $? -ne 0 ]
then
	echo -e "\e[0;31m[!] Upload static files\\r"
	bail "$STATIC_STATUS"
fi
echo -e "\e[0;32m[X] Upload static files\\r"
echo -ne "\e[0m"


if [ "$1" == "static" ]; then
	exit 0
fi

echo -ne "\e[0;37m[ ] (re)Start the espresso service\\r"
SERVICE_STATUS=$(ssh -i ~/.ssh/id_donny_nyc root@104.131.30.208 -C "systemctl restart espresso" 2>&1)
if [ $? -ne 0 ]
then
  echo -e "\e[0;31m[!] (re)Start the espresso service"
  bail "$SERVICE_STATUS"
fi
echo -e "\e[0;32m[X] (re)Start the espresso service"

echo -ne "\e[0m"

echo -ne "\e[0;37m[ ] espresso service ok\\r"
ESPRESSO_STATUS=$(ssh -i ~/.ssh/id_donny_nyc root@104.131.30.208 -C "systemctl status espresso" 2>&1)
if [ $? -ne 0 ]
then
	echo -e "\e[0;31m[!] espresso service ok"
	bail "$ESPRESSO_STATUS"
fi

echo -e "\e[0;32m[X] espresso service ok"

echo -ne "\e[0m"
