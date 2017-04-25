#ipcamera-manage-tool-for-linux
## 1、下载gsoap
[gsoap-2.8.44下载地址](https://nchc.dl.sourceforge.net/project/gsoap2/gsoap-2.8/gsoap_2.8.44.zip)

## 2、编译gsoap
* 解压gsoap_2.8.44.zip
* cd 解压gsoap_2.8.44
* ./configure
* make
* make install

## 3、生成onvif框架代码请参考

* https://www.genivia.com/resources.html#How_do_I_use_gSOAP_with_the_ONVIF_specifications?
* 

## 4、下面是我生成相关代码框架的命令

```
wsdl2h -o onvif.h -c -s -t ./typemap.dat http://www.onvif.org/onvif/ver10/device/wsdl/devicemgmt.wsdl http://www.onvif.org/onvif/ver10/events/wsdl/event.wsdl http://www.onvif.org/onvif/ver10/display.wsdl http://www.onvif.org/onvif/ver10/deviceio.wsdl http://www.onvif.org/onvif/ver20/imaging/wsdl/imaging.wsdl http://www.onvif.org/onvif/ver10/media/wsdl/media.wsdl http://www.onvif.org/onvif/ver20/ptz/wsdl/ptz.wsdl http://www.onvif.org/onvif/ver10/receiver.wsdl http://www.onvif.org/onvif/ver10/recording.wsdl http://www.onvif.org/onvif/ver10/search.wsdl http://www.onvif.org/onvif/ver10/network/wsdl/remotediscovery.wsdl http://www.onvif.org/onvif/ver10/replay.wsdl http://www.onvif.org/onvif/ver20/analytics/wsdl/analytics.wsdl http://www.onvif.org/onvif/ver10/analyticsdevice.wsdl http://www.onvif.org/ver10/actionengine.wsdl http://www.onvif.org/ver10/pacs/accesscontrol.wsdl http://www.onvif.org/ver10/pacs/doorcontrol.wsdl http://www.onvif.org/ver10/advancedsecurity/wsdl/advancedsecurity.wsdl http://www.onvif.org/onvif/ver10/schema/onvif.xsd 

soapcpp2  -c onvif.h -x -I ../gsoap/gsoap-2.8/gsoap/import -I ../gsoap/gsoap-2.8/gsoap/custom -I ../gsoap/gsoap-2.8/gsoap/extras -I ../gsoap/gsoap-2.8/gsoap/plugin -I ../gsoap/gsoap-2.8/gsoap

```
