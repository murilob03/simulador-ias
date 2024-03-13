# Simulador IAS

O Simulador IAS é uma ferramenta que simula o funcionamento da máquina IAS desenvolvida por 	John von Neumann no "Institute for Advanced Study" (IAS). Este projeto permite que você compile e execute programas escritos em linguagem de montagem IAS, proporcionando uma visão detalhada de como o processador IAS executa as instruções.

## Como compilar

Para compilar o projeto, basta executar o comando:

```bash
$ make
```

Este comando compilará o código-fonte do simulador, produzindo o executável `ias`.

## Como rodar

Para executar o simulador, utilize o seguinte comando:

```bash
$ ./ias -p <entrada.ias> -i <valor_para_pc>
```

Substitua `<entrada.ias>` pelo arquivo contendo o programa em linguagem de montagem IAS que você deseja executar e `<valor_para_pc>` pelo valor inicial do contador de programa (PC).

## Arquivo de entrada

O arquivo de entrada (`entrada.ias`) contém o programa a ser executado pelo simulador. Na pasta raiz do projeto, você encontrará um arquivo de exemplo chamado `exemplo.ias`, que demonstra todas as operações possíveis suportadas pela linguagem de montagem IAS.

É permitido escrever tanto números quanto instruções em todas as linhas do arquivo de entrada. No entanto, não são permitidas linhas em branco. Caso haja linhas em branco, estas devem ser completadas com zeros.

Por exemplo:

```
load m(0)
load mq,m(10)
add m(4)
jump m(5,0:19)
0
0
stor m(0)
exit
```



## Configuração da quantidade de ciclos por operação

É possível configurar a quantidade de ciclos por operação para simular diferentes cenários de desempenho. Essa configuração é opcional e deve ser feita em um bloco de comentário delimitado por `/*` e `*/`, localizado no início do arquivo, antes de qualquer instrução ou comentário. As configurações possíveis estão detalhadas no arquivo `exemplo.ias`.

