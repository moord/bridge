Пример использования

param.json      - пример структуры динамического массива

device_emulator - эмулятор "управляющего контроллера технологической линии"

- раз в 5 секунд заменяет значения аналоговых датчиков случайным числом
- значения управляющих сигналов копирует в состояния с таким жен названием (например управление Нория 1 -> состояние Нория 1 )

device_emulator <последовательный порт> <файл со структурой динамического массива>

Пример использования:
                     
socat -d -d pty,rawer,echo=0 pty,rawer,echo=0
	- создаст пару связанных виртуальных портов. Например  /dev/pts/1 и /dev/pts/2

./device_emulator /dev/pts/1 param.json
	- запуск эмулятора "управляющего контроллера" 

./bridge -s /dev/pts/2 -F param.json -L file --port1 9000 -2 9002
	- запуск основной программы 

nc localhost 9002 
	- пример контроля изменений по подписке
	- для задания управляющего сигнала можно отправить команду MANAGE 1 1 (MANAGE <index> <value[0|1]>)

vu_example_win32.exe  - приложение win32 с примером окна монитринга и управления (подключаться к порту указанному в --port1  по умолчанию 9000)
   