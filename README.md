# helloworld-native-application

A application using binding example for AGL

## Setup 

```bash
git clone --recursive https://github.com/iotbzh/helloworld-native-application.git
cd helloworld-native-application
```

## Build  for AGL

```bash
#setup your build environement
. /xdt/sdk/environment-setup-aarch64-agl-linux
#build your application
./autobuild/agl/autobuild package
```

## Build for 'native' Linux distros (Fedora, openSUSE, Debian, Ubuntu, ...)

```bash
./autobuild/linux/autobuild package
```

## Deploy

### AGL

```bash
export YOUR_BOARD_IP=192.168.1.X
export APP_NAME=helloworld-native-application
scp build/${APP_NAME}.wgt root@${YOUR_BOARD_IP}:/tmp
#install the widget
ssh root@${YOUR_BOARD_IP} afm-util install /tmp/${APP_NAME}.wgt
APP_VERSION=$(ssh root@${YOUR_BOARD_IP} afm-util list | grep ${APP_NAME}@ | cut -d"\"" -f4| cut -d"@" -f2)
#start the binder
ssh root@${YOUR_BOARD_IP} afm-util start ${APP_NAME}@${APP_VERSION}
```

## TEST

### AGL

```bash
export YOUR_BOARD_IP=192.168.1.X

#you can display the log from systemd journal
ssh root@${YOUR_BOARD_IP} journalctl -f

#you can display log from file
ssh root@${YOUR_BOARD_IP} cat /tmp/helloworld.log

#you can display the binder status
ssh root@${YOUR_BOARD_IP} afm-util ps

#you can stop the binder by remove file **helloworld.log**
ssh root@${YOUR_BOARD_IP} rm /tmp/helloworld.log
```

# Activate authentification security

The security is actived in file **conf.d/wgt/config.xml.in** by:

```xml
  <feature name="urn:AGL:widget:required-permission">
  <param name="urn:AGL:permission:monitor:public:get" value="required" />
  </feature>
```

To disable security

* remove the feature section named **urn:AGL:widget:required-permission** from the xml file **conf.d/wgt/config.xml.in**
* rebuild your application
