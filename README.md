# simple-webserver-c
Project Terkait Pembuatan Webserver C, project ini ditujukkansebagai pembelajaran terkait Sistem Operasi, webserver ini dibuat untuk mendukung kebutuhan multiclient, IPC seperti fork dan semaphore digunakan untuk penanganan multiclient.

## Compile & Decompile Program
### Compile
```shell
make
```
### Decompile
```shell
make clean
```


## Menjalankan Server
```shell
./server.o
```
## Format Request Client yang didukung
### GET Request
```shell
 curl http://localhost:6969/
```
### POST Request
```shell
curl -X POST http://localhost:6969/echo -H "sigmabot" -d 'sigma' 
```
### PUT Request
```shell
curl -X PUT http://localhost:6969/sigma.txt -d 'text 123'
```
### DELETE Request
```shell
curl -X DELETE http://localhost:6969/sigma.txt
```

## Testing Multiclient
### Test Multiclient - GET Request
```shell
./test/test_multiclient_GET.sh
```
### Test Multiclient - POST Request
```shell
./test/test_multiclient_POST.sh
```
### Test Multiclient - PUT Request
```shell
./test/test_multiclient_PUT.sh
```

# Reference
https://github.com/infraredCoding/cerveur.git
