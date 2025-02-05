# Parallel prefix sum
Программа позволяет параллельно подсчитать префиксную сумму на процессоре/видеокарте/встроенной видеокарте.

# Входные аргументы:
1) --input: входной файл
2) --output: файл, в который будет производиться запись
3) --device-type: тип устройства, на котором будет запущено вычисление. Варианты: {dgpu, igpu, cpu, gpu, all} 
4) --device-index: i-ое устройство типа --device-type, на котором будет запущено вычисление

# Пример использования:
## input.txt
```
10
1 2 3 4 5 6 7 8 9 10
```
## Запуск программы
```bash
./app --input input.txt --output output.txt --device-type dgpu --device-type 0
```

## Вывод в консоль:
```bash
Device: NVIDIA GeForce RTX 3060 Laptop GPU      Platform: NVIDIA CUDA
Time: 0.065536  0.069376
LOCAL_WORK_SIZE [32, 1]
```
##  output.txt
```
1.000000 3.000000 6.000000 10.000000 15.000000 21.000000 28.000000 36.000000 45.000000 55.000000
```
