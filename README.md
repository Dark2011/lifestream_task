# Human Machine Interface #

## Структура проекта

[HMI Structure](doc/HmiStructure.md)

## FRS 

[Оглавление](doc/FRS/FRSContents.md)

[HTML TREE](doc/FRS/tree_head.html)

## Технические требования к продуктам HMI


### HMI ADMIN
[Технические требования к утилите HMIAdmin](doc/HMIAdmin/HmiAdmin.md)

## Общее описание архитектуры и алгоритмов
[Описание протокола RCSProtocol](/doc/RCSProtocol/RCSProtocol.md)<br>
[Алгоритм реализации ответственных команд](/doc/hmi/CriticalCommandsSM.md)<br>

[CPU. Описание архитектуры ЦП](/doc/CPU/cpu.md)<br>
[HMIDesigner. ReleaseNotes](/doc/HMIDesigner/ReleaseNotes.md)<br>


# Соглашение по Code Style 

[Cоглашение по Code Style](/doc/InternalDocs/codeStyleConventions.md)

## Глоссарий
[Глоссарий](/doc/InternalDocs/glossary.md)

## Инструкция по генерации переводов
[Инструкция по генерации переводов](/doc/InternalDocs/translationGenerationInstruction.md)

## Инструкция по генерации client-data.hmitar
[Инструкция по генерации Client Data](/doc/InternalDocs/howToUpdateClientDataHmitar.md)
## Build for Android
[Инструкция по сборке под Android](doc/InternalDocs/howToGenerateAndroid.md)

## Как настроить отображение виджетов
[Инструкция по настройке виджетов](doc/hmi/ToolbarSettings.md)


## Сборка проекта ##

### Build for Visual Studio 2017
```sh
mkdir build
cd build
cmake .. -G "Visual Studio 15 2017 Win64"
```

### Build for Linux
Имеется 2 варианта сборки проекта
1. Полная (Зависимости Qt 5.7+ C++14)
2. Только клиент (Loger RCSProtocol hmiClient) (Зависимости Qt 5+ C++11)

Для полной сборки
```sh
mkdir build
cd build
cmake .. -DQT_DIR=/opt/Qt5.9.4/5.9.4/gcc_64
make
```

или

```sh
mkdir build
cd build
cmake .. -DQT_DIR=/opt/Qt5.9.4/5.9.4/gcc_64
cmake --build . --target all
cmake --build . --target clean
```

Для сборки отдельного проекта сборка указывается через cmake,
при этом будут собираться и библиотеки, от которых зависит проект.
```sh
mkdir build
cd build
cmake .. -DQT_DIR=/opt/Qt5.9.4/5.9.4/gcc_64
cmake --build . --target hmiClient
cmake --build . --target clean
```

-DQT_DIR=/opt/Qt5.9.4/5.9.4/gcc_64 - указывается если была установлена внешняя Qt
