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
