# Descripción del programa

Este programa implementa un manejador de tipos de datos. El sistema es capaz de gestionar **tipos atómicos**, **registros** y **registros variantes**, permitiendo simular la definición de tipos simples y compuestos. Imprime la disposición en memoria de los datos utilizando estrategias de no empaquetamiento, empaquetamiento y reordenamiento de campos utilizando heurísticas.
# Instrucciones de ejecución

Para ejecutar el programa, se requiere tener instalados el compilador **g++** y la librería **lcov**. La instalación puede realizarse con:

```bash
sudo apt install g++
sudo apt install lcov
```
Para ejecutar el programa principal (Type-manager.cpp):
```bash
make run
```

Para ejecutar las pruebas:
```bash
make tests
```

Para generar el reporte de cobertura:
```bash
make tests
```

Para limpiar el directorio fuente:
```bash
make clean
```
