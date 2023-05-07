## Jackalope storage module

Made by Egoshin A. (ego)

### Разработка

Для удобства разработки используйте отладку на удаленном docker-контейнере (по идее любая современная IDE 
поддерживает такую фичу). Идея "удаленной отладки" следующая:

1. Запускаем sleep-infinity docker-контейнер, который эмулирует тестовое окружение (Ubuntu 20.04 + deps):
```bash
docker build -t jackalope-debug-container -f Dockerfile .
docker run -d --name jackalope-debug -p 42222:22 --cap-add sys_ptrace \
  --security-opt seccomp=unconfined --security-opt apparmor=unconfined \
  jackalope-debug-container
```
2. Подключаемся из IDE к debug-контейнеру, используя креды:
    - **host**: localhost
    - **port**: 42222
    - **user**: debugger
    - **password**: pwd
3. (**Только для CLion**) Конфигурим IDE:
    - **toolchain**: File -> Settings -> Build, Execution, Deployment -> Toolchains -> Remote Host
        - заполняем креды из пункта 2
    - **deployment**: File -> Settings -> Build, Execution, Deployment -> Deployment
        - заполняем инфу про путь, по которому CLion будет заливать файлы в docker-контейнер
    - **cmake**: File -> Settings -> Build, Execution, Deployment -> CMake
        - оставляем поля по умолчанию, однако `build directory` указывает как `build`