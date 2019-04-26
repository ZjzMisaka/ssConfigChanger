# ssConfigChanger
### 主要功能
· 本软件是用于一键从服务器获取最新可用端口并更新本地ss端口设置的程序. <br/>
· 指定存有端口号信息的页面, 从页面上获取最新端口号, 更改指定的ss配置项, 并重启ss. <br/>
### 操作指南
#### 配置文件
· 默认的配置文件路径在程序的同级目录下, 没有会自动创建 不出意外全程不需要手动编辑. <br/>
· 默认的ss路径为程序根目录的上一级目录, 因此可以把程序目录放在ss根目录下. <br/>
· 初次打开程序, 如果没有现成的配置文件, 需要手动填写端口获取页面地址与服务器地址, 如果程序不在ss目录下则需要指定路径. 这些设置会保存在配置文件中, 不出意外不用手动改动. <br/>
#### 获取端口
· 配置文件完整的情况下, 如果需要获取最新端口, 只需要在主界面或系统托盘菜单点击一下按钮即可. <br/>
#### 其他
· 可以在系统托盘进行获取端口与重启ss等操作. <br/>
· 可以设置开机自启动. <br/>
### 对于服务器
· 服务器方需要进行的操作为将更换的端口数字放在可通过http协议在外网访问的网站中的文件上, 文件名与后缀不限, 文件内容为纯数字 (端口值), 不包含代码等任何其他内容. <br/>
**例如: 使用crontab定时执行脚本**
````
#!/bin/bash
PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin:~/bin
export PATH
htmlName=/xxxxxxxx/ShadowSocksPort.html
now=`cat $htmlName`
echo $now
temp=$(python -c 'import random;print random.randint(xxxx,xxxx)')
echo $temp
sed -i "s/$now/$temp/" /etc/ShadowSocks/ssConfig.json
sed -i "s/$now/$temp/" /xxxxxxxx/ShadowSocksPort.html
ssserver -c /etc/ShadowSocks/ssConfig.json -d restart
````
· 由于默认路径放在ss根目录下, 并且可以操作ss的启动, 关闭等基本操作, 因此可以当作启动器使用. 可以配好配置文件一并打包. <br/>
