# Kompilator prostego języka imperatywnego

## Autor: Szymon Rzewuski
numer indeksu: 272341

## Opis

Projekt kompilatora prostego języka imperatywnego na maszynę wirtualną z prostym językiem assemblera, autorstwa profesora Maćka Gębali, stworzony z pomocą narzędzi flex oraz bison.

### Zawarte pliki

* src/
    * lexel.l - plik flex
    * parser.y -plik bison
    * symbol_table.hpp - prosta biblioteka do obsługi symboli
    * symbol_table.cpp - implementacja symbol_table.hpp
    * utis.cpp - główny plik z funkcjonalnościami kompilatora
    * utils.hpp - funkcje wystawione do użytku przez bison
* Makefile - plik budujący projekt

## Wymagane programy

* flex 2.6.4
* bison (GNU Bison) 3.8.2
* g++ 11.4.0
* GNU Make 4.3

## Instrukcja

Projekt należy zbudować poleceniem
```
make
``` 
stworzony zostanie plik wynikowy o nazwie **kompilator**, wywołanie programu następuje w poniższy sposób:

```
./kompilator <plik wejściowy> <plik wyjściowy>
```

Aby wyczyścić zbudowany projekt, należy wywołać polecenie

```
make clean
```

