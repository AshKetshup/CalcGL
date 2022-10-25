### **Universidade da Beira Interior**

### Departamento de Informática

![image](ubi-fe-di.png){width="191pt"}

### **N 121 --- 2022**\
# ***Visualização de funções implícitas por *ray marching****

#### Elaborado por:
## **Diogo Castanheira Simões**

#### Orientador:
### **Professor Doutor Abel João Padrão Gomes**

2022-10-25

# Agradecimentos

Em primeiríssimo lugar, agradeço à minha família que independentemente
da situação sempre se demonstraram dispostos a me apoiar durante todo o
meu percurso académico. Em destaque aos meus pais, pois foram eles que
permitiram que tenha chegado onde atualmente estou.

Agradeço ao meu professor orientador, Prof. Doutor Abel João Padrão
Gomes por ter aceitado o desafio em me orientar e pela confiança
depositada em mim durante esta jornada.

Aos meus dois melhores amigos, Raquel Guerra e Igor Nunes, estou
eternamente grato pelo apoio incondicional dado desde o dia em que nos
conhecemos. Sem as suas amizades certamente não seria a pessoa que sou
hoje.

Ao Igor Nunes, quero ainda demonstrar a minha gratidão pelo suporte dado
durante o desenvolvimento deste projeto, atuando como um *quasi*
coorientador.

Ao meu amigo Pedro Cavaleiro pelas noitadas de boa disposição e
companhia assim como as refrescantes subidas à Serra.

Não esquecendo da companhia e camaradagem dos meus amigos Cristiano
Santos e Pedro Batista.

Em último, mas não menos importante, um agradecimento aos meus restante
amigos que, mesmo sem serem aqui mencionados por nome, estiveram sempre
disponíveis para um café e conversa.

# Introdução

## Enquadramento

O processo de síntese de imagem em computação gráfica é na sua essência
a transformação de geometria em pixeis. As duas técnicas mais usuais de
o fazer são: renderização e *ray casting*. Na renderização, a geometria
é dada por uma malha de triângulos e é o próprio sistema gráfico (e.g.,
*OpenGL* ou *DirectX*) que transforma os triângulos em pixeis. No *ray
casting*, o processo é diferente pois emitem-se raios (um por pixel) a
partir da câmara (i.e., o olho do observador) numa cena, calculando-se
então os pontos de intersecção entre os raios e a geometria de cada
objeto existente na cena. No caso presente, utilizou-se o *ray
marching*, que é uma variante do *ray casting*.

Obviamente, a geometria de cada objeto tem uma dada representação
matemática. As representações matemáticas da geometria destes objetos
são dadas por funções analíticas. As representações são de três tipos:
explícitas, paramétricas e implícitas. No trabalho descrito neste
relatório, utilizaram-se funções implícitas para descrever a geometria
de cada objeto.

## Motivação

As versões modernas do *OpenGL* (i.e. a partir da versão 3.3) permitem
que o programador escreva os seus próprios *shaders* a serem utilizados
na *pipeline* de renderização. Uma consequência imediata é a
possibilidade de paralelizar inúmeros cálculos que normalmente seriam
realizados pela CPU. Ainda assim, uma miríade de *software*
de renderização não tira proveito de tais capacidades. Desta forma, é de
interesse estudar um algoritmo de renderização por volume e analisar a
sua potencial paralelização.

## Objetivos

O presente projeto tem por **objetivo principal** implementar um sistema
de visualização de funções implícitas.

Este tem ainda os seguintes **objetivos secundários**:

-   Estudar funções implícitas e o cálculo das respetivas
    iso-superfícies;

-   Analisar o modelo de renderização por volume;

-   Implementar o algoritmo de *ray marching* em particular;

-   Implementar aceleração por *hardware* através da programação de
    *shaders* em *OpenGL*.

-   Avaliar as diferenças entre CPU *single-threaded*, CPU *multi-threaded* e acelerado por GPU.

## Organização do Documento {#sec::intro:orgdoc}

O presente relatório estrutura-se em seis capítulos:

1.  No primeiro capítulo --- **Introdução** --- é apresentado o projeto,
    em particular o seu enquadramento e motivação, assim como os seus
    objetivos e a respetiva organização do relatório.

2.  No segundo capítulo --- **Estado da Arte** --- são apresentados os
    conceitos fundamentais de funções implícitas, assim como métodos
    usados para renderizar, com particular foco no algoritmo *Ray
    Marching*. É também feita uma introdução aos conceitos introdutórios
    do *OpenGL* moderno.

3.  No terceiro capítulo --- **Tecnologias e Ferramentas** --- são
    definidos os conjuntos de tecnologias, ferramentas e *hardware*
    usados para a implementação do projeto.

4.  No quarto capítulo --- **Implementação** --- são descritas as
    decisões e detalhes tomados durante o desenvolvimento, em conjunto
    com o funcionamento do programa e os seus requisitos.

5.  No quinto capítulo --- **Testes e Resultados** --- é demonstrado o
    funcionamento do programa final, os resultados por si obtidos e as
    diferentes fases de teste.

6.  No sexto capítulo --- **Conclusões e Trabalho Futuro** --- é feita a
    reflexão sobre os conhecimentos obtidos ao longo do desenvolvimento
    do projeto, os seus obstáculos, o que não foi possível alcançar e o
    que poderá ser explorado no futuro.

# Estado da Arte {#ch::arte}

A visualização computacional de funções é a base de muitas aplicações
práticas em computação gráfica, tais como em videojogos e visualização
molecular. O estudo destas funções permite igualmente outro tipo de
cálculos, tais como colisões.

Para alcançar os objetivos propostos no presente projeto, é imperativo
estudar os seguintes tópicos:

-   Funções implícitas;

-   Técnicas de renderização em geral e algoritmos volumétricos em
    particular;

-   Uso da API *OpenGL* e programação em GLSL.

## Funções Implícitas {#sec::arte:implicitas}

Às funções definidas em função de uma variável dá-se o nome de **funções
explícitas**. Exemplos clássicos em $\mathbb{R}^2$ incluem equações de
retas ($y = mx + b$) e parábolas ($y = ax^2 + bx + c$). Ora, nem todos
os subconjuntos de pontos no espaço cartesiano podem ser definidos por
funções explícitas. Um exemplo comum em $\mathbb{R}^2$ é a
circunferência:

$$(x - x_0)^2 + (y - y_0)^2 = r^2
	\label{eq::circ_implicita}$$

onde $(x_0, y_0)$ é o centro e $r$ o raio. Exemplifica-se na Figura
2.1 a circunferência definida por $(x - 1)^2 + (y - 2)^2 = 4$.

![Exemplo de uma circunferência de centro $(1, 2)$ e raio
$2$.](circumference)

Sendo uma equação de segundo grau, é possível representá-la através de
duas funções explícitas:

$$\begin{aligned}
		y = y_0 + \sqrt{r^2 - (x - x_0)^2} \\
		y = y_0 - \sqrt{r^2 - (x - x_0)^2}
\end{aligned}$$

Esta transformação não é possível para todos os polinómios, em
particular para graus superiores a $4$. Contudo, estas expressões
polinomiais continuam a ser subconjuntos válidos de $\mathbb{R}^n$,
necessitando então de formas alternativas de representação. Dois métodos
e respetivas representações da circunferência são:

1.  **Funções paramétricas**: cada eixo é definido em ordem a uma
    variável adicional $t$: $$\left\{\begin{array}{l}
    			x = r\cos(t) \\
    			y = r\sin(t)
    		\end{array}\right.
    	\label{eq::circ_parametrica}$$

2.  **Funções implícitas**: a equação não é definida a ordem a uma
    variável em particular.

Uma **função implícita** é então definida por
$f~:~\mathbb{R}^n \longrightarrow \mathbb{R}$, ou seja, para qualquer
ponto em $\mathbb{R}^n$ é determinado um resultado em $\mathbb{R}$.
Dependendo da função, o valor obtido pode ter significado, tal como uma
grandeza física (*e.g.* densidade de um líquido ou sua temperatura a
cada ponto do espaço). Esta função diz-se **algébrica** caso seja
polinomial em cada variável.

Por seu turno, em $\mathbb{R}^3$, a **iso-superfície** de uma função
implícita é a superfície que satisfaz a condição $f(\mathbf{x}) = 0$
(onde, doravante, $\mathbf{x} \equiv (x,y,z)$). Esta pode ser suavizada
através de um parâmetro $s \in \mathbb{R}$ tal que:

$$f(\mathbf{x}) - s = 0
	\label{eq::suavizacao}$$

Este efeito é utilizado diretamente e com sucesso nas Superfícies $\Pi$.


## Renderização de Superfícies Implícitas {#sec::arte:render}

Muitos algoritmos de renderização têm vindo a ser investigados e usados
em *software* que implemente diversas técnicas para obter uma imagem
final.

A nossa visão capta a luz refletida por objetos num determinado cenário.
Durante o processo, uma parte dos comprimentos de onda das fontes de luz
são absorvidos pelos objetos, sendo a restante refletida. Como resultado
final, os comprimentos de onda refletidos e captados pelos olhos
constituem o que é interpretado pelo cérebro como **cor**.

No entanto, a determinação do percurso de cada partícula de luz (ou
fotão) num dado cenário é na grande maioria dos casos algo impraticável
devido ao volume de cálculos envolvidos, com a consequente demora na
obtenção dos resultados.

Neste sentido, foram desenvolvidos métodos mais eficientes de cálculo,
tais como:

-   **Rasterização**: técnica ainda usada na *pipeline* do *OpenGL*,
    consiste na conversão de uma imagem descrita geometricamente em 3D
    numa série de pixeis que, no seu todo, criam a sua imagem
    representativa em 2D;

-   ***Ray Casting***: considerando um cenário a ser observado a partir
    de um determinado ponto de vista, este algoritmo calcula a imagem
    observada com base na geometria e em leis óticas fundamentais;

-   ***Ray Tracing***: funcionalmente semelhante ao *Ray Casting*,
    recorre a simulações óticas mais avançadas para a criação de
    cenários mais realistas.

Neste âmbito, as funções implícitas representam simultaneamente uma
grande oportunidade e um enorme desafio na área da computação gráfica.
Se por um lado é possível obter a visualização de formas geométricas
complexas com recurso a funções implícitas, por outro não há um método
direto geral para determinar os pontos da iso-superfície. Vários
algoritmos computacionais para este fim têm sido propostos ao longo das
últimas décadas. Para fins do presente Estado da Arte, estes podem ser
categorizados em dois grandes grupos:

1.  **Triangulação**: A iso-superfície é dividida em triângulos, os
    quais formam a *mesh* a ser renderizada pela GPU. Este é
    considerado o algoritmo clássico e de eleição:

    -   *Marching cubes*: O espaço é dividido em cubos
        onde o valor da função implícita é calculado para cada vértice.
        A análise dos sinais permite determinar quais arestas do cubo a
        superfície interseta, num total de 256 possíveis combinações de
        triângulos.

2.  **Algoritmos volumétricos**: Permitem determinar a iso-superfície
    através do cálculo de interseções. Exemplos incluem:

    -   *Ray marching*: Uma série finita de raios com origem no
        observador são emitidos para a cena a ser visualizada, e a
        interseção é estimada iterando sobre um ponto que marcha nesse
        raio.

    -   *Sphere tracing*: Um caso particular de *ray
        marching* onde esferas são usadas para determinar a distância
        mínima aos objetos do cenário.

    -   *Ray tracing* (previamente descrito).

## *Ray Marching* {#sec::arte:raymarch}

Um método de estimar pontos da iso-superfície de uma função implícita
passa pela determinação da sua interseção com retas. Pelo princípio
tomado nos métodos da família de *ray casting*, um raio com origem no
observador e um sentido na cena a ser observada é considerado.
Diferentes algoritmos podem ser utilizados para estimar o ponto de
interseção (ou calcular com precisão se tal for possível). No caso de
*ray marching*, a interseção do raio com a superfície implícita é feita
colocando um ponto em "marcha" ao longo do primeiro.

### *Ray Casting* {#ssec::arte:raymarch:cast}

Seguir-se-á uma explicação do processo de *ray casting* tendo por base o
esquema da Figura 2.2.

Considere-se então os seguintes parâmetros relativos ao **observador**
(ou câmara):

-   **Origem** no ponto $C$;

-   **Vetores de visualização** normalizados:

    -   Frontal (*front*), $\overrightarrow{f}$, na direção do eixo $z$;

    -   Direito (*right*), $\overrightarrow{r}$, na direção do eixo $x$;

    -   Superior (*up*), $\overrightarrow{u}$, da direção do eixo $y$.

-   **FOV**, dado por $s$.

![Esquema representativo do processo de *ray casting*. A partir do
observador $C$, com os respetivos vetores de visualização
$\overrightarrow{f}$, $\overrightarrow{r}$ e $\overrightarrow{u}$, e o
FOV definido pela distância $s$, determina-se o plano de visualização $\varphi$. 
Os raios emitidos, $a$, interceptam $\varphi$ no ponto $A$, o qual está associado
a um fragmento no *OpenGL*.

O ponto $P$ a partir do qual se definirá o plano de visualização é
definido por:

$$P = C + s \cdot \overrightarrow{f}
	\label{eq::cast:pontoP}$$

Desta forma, o **plano de visualização** $\varphi$ é definido com base
no ponto $P$ e nos vetores $\overrightarrow{r}$ e $\overrightarrow{u}$:

$$\varphi ~\equiv~ (x,y,z) = P + x \cdot \overrightarrow{r} + y \cdot \overrightarrow{u}
	\label{eq::cast:plano}$$

Desta forma, os **raios** emitidos a partir do observador, $a$,
interceptam o plano de visualização $\varphi$ no ponto
$A(x_A, y_A, z_P)$, e são definidos pela seguinte equação vetorial:

$$a ~\equiv~ (x,y,z) = C + w \cdot (A - C)
	\label{eq::cast:ray}$$

Através de um processo de *ray marching* pode-se estimar o valor $w$ que
determina o ponto de intercepção $I$. A distância a que se encontra do
observador, em conjunto com o vetor normal em $I$, permitem calcular a
cor do fragmento (geralmente um pixel) associado ao ponto $A$.

### Algoritmo naïve

No algoritmo naïve, um ponto é iterado sobre o raio emitido num processo
de "marcha". A cada ponto o valor da função implícita é calculado.
Assumindo que esta é contínua em $\mathbb{R}^3$, o Teorema de
Bolzano-Cauchy pode ser utilizado para determinar se ocorreu uma
interseção entre dois pontos de marcha contíguos (Figura 2.3).

![Demonstração do algoritmo *Ray Marching* em 2D. Um ponto (a vermelho)
"marcha" ao longo de um raio emitido no sentido da cena com origem no
observador até ser detetada uma interseção com a função implícita
(verde). O Teorema de Bolzano-Cauchy indica que tal interseção existe
entre o último ponto vermelho e o ponto
azul.](raymarch2D)

A fim de aumentar a precisão do ponto de interseção estimado, pode-se
proceder a uma marcha em sentido inverso com distâncias menores a cada
iteração até se chegar a um ponto suficientemente perto da função
implícita que se possa considerar o ponto de interseção. A este processo
dá-se o nome de *bisection* (Figura
2.4).

Esta técnica é de particular interesse nos seguintes casos:

-   Necessidade de renderizar volumes que não são uniformes;

-   Renderização de funções implícitas ou fractais;

-   Renderização de outros tipos de funções paramétricas onde a
    interseção não é conhecida antecipadamente, como *parallel mapping*.

Contudo, este é um algoritmo geral que se pode aplicar a qualquer função
tridimensional (Figura 2.5).

![Processo de *bisection* usado no algoritmo naïve de *ray
marching*.](bisection)

![Teste do algoritmo naïve no *website* [shadertoy.com](shadertoy.com) 
com uma esfera.](ashalgosphere)

### *Sphere Tracing* {#ssec::arte:raymarch:spheretracing}

*Sphere tracing* é uma técnica que faz uso do algoritmo de *ray
marching* para renderizar **superfícies implícitas** conhecidas (Figura
[2.6](#fig::spheresphere){reference-type="ref"
reference="fig::spheresphere"}). É **requisito necessário** ter
conhecimento antecipado da distância mínima entre um dado ponto e a
superfície em questão. Esta distância é determinada com funções
denominadas de **SDF** (Figura 2.7). O uso das mesmas torna impraticável 
a aplicação de *sphere tracing* para funções implícitas de geometria 
desconhecida.

Visto que uma função SDF pode ser resolvida para cada ponto,
podemos determinar a maior esfera possível que possa caber no atual
passo de marcha e cujo raio seja a distância mínima entre o ponto e a
superfície. Sabemos então que a próxima distância a "marchar" é o raio
desta esfera (Figura 2.8). Tal permite optimizar o número de passos
necessário na "marcha", aumentando significativamente a eficiência do
processo de *ray marching*.

![Teste do algoritmo de *sphere tracing* no *website*
[shadertoy.com](shadertoy.com) de uma esfera e um
plano.](spheresphere)

![Visualização da SDF para a circunferência $x^2 + y^2 - 1$. A
escala de cinzas é proporcional à distância à superfície, sendo preto em
valores negativos.](sdf02)

![Demonstração do algoritmo *Sphere Tracing* em 2D. Cada novo ponto de
"marcha" é distancia-se do anterior a uma distância igual ao raio da
esfera que satisfaz a função SDF para a superfície
visualizada.](stracing2D)

## *OpenGL*

Pela década de 1980, o processo de desenvolvimento de *software* que
funcionasse em diferentes tipos de *hardware* gráfico era algo
desafiante pela grande falta de *standards*. No início da década
seguinte, a *Silicon Graphics* (SGI) era a líder em *workstations* de
gráficos 3D, e o seu *IrisGL* tornou-se o *de facto standard* da
indústria.

Entretanto apareceu o *OpenGL* como uma alternativa ao *IrisGL*, uma
API que nos providencia um vasto conjunto de funcionalidades que podemos 
usar para manipular imagens e gráficos. Por si só o *OpenGL* não é 
apenas uma API, mas sim uma especificação. Desenvolvida e mantida pela 
*Khronos Group*, é encarregue de especificar o resultado de cada função 
e como a mesma se deve comportar para um determinado *hardware*. Cabe 
aos fabricantes implementar *drivers* que cumpram este *standard*.

*OpenGL* é essencialmente uma grande ***state machine***, i.e. uma
coletânea de variáveis que descreve como o *OpenGL* se deverá comportar
a cada instante. Durante o seu uso é alterado o seu estado definindo
opções, *buffers* e, assim, renderizando usando o atual contexto.

### *Pipeline*

A renderização de um dado objeto em *OpenGL* passa por diversos passos,
a cujo conjunto é dado o nome de ***pipeline* de renderização**. A
partir da versão 3.3, intitulado de "*OpenGL* moderno", a *pipeline*
possui determinados pontos onde o programador tem a capacidade de
programar diretamente para a GPU através de pequenos programas compostos
por *shaders*.

A *pipeline* é executada sempre que uma operação de renderização for
iniciada. Estas operações requerem o uso obrigatório de um
VAO propriamente definido, em conjunto com um *Program Object* ou um 
*Program Pipeline Object*, o qual fornece os *shaders* para certos pontos 
da *pipeline*.

Das fases englobadas pela *pipeline*, destacam-se cinco:

1.  **Processamento de vértices** (*vertex processing*): Esta fase é
    programável em três pontos consecutivos, o que abre uma vasta margem
    de customização ao programador sobre como os vértices são
    processados. Após a especificação do vértice, este é processado por
    um *vertex shader* tornando-o num vértice de saída. Opcionalmente
    pode-se manipular o processo seguinte, de tesselação (*tessellation
    stage*), assim como processamento de primitivas com o uso de um
    *geometry shader*, igualmente opcional.

2.  **Pós-processamento de vértices** (*vertex post-processing*):
    Durante este processo os vértices disponíveis são filtrados pela
    condição de serem compreendidos no *viewport* atual. Os mesmos
    também poderão ser transportados para diferentes localizações no
    espaço.

3.  **Rasterização** (*rasterization*): É neste processo que as
    primitivas são divididas em elementos independentes designados de
    **fragmentos**. Por norma, a cada pixel é associado um fragmento.

4.  **Processamento de Fragmentos**: Para cada fragmento é gerado uma
    quantia de *outputs* (por exemplo a cor), processados por um
    *fragment shader*, cuja customização pelo programador é opcional.

5.  **Processamento por Amostra** (*per-sample operations*): Com os
    resultados devolvidos pelo *fragment shader*, os mesmos passam por
    diversos testes (por exemplo, de profundidade), resultando então num
    pixel final.

### *Shaders* {#ssec::arte:opengl:shaders}

Aos sub-programas executados pela GPU num determinado ponto da *pipeline* de
renderização chamamos de *shaders*. Um conjunto de *shaders* constitui
um **programa**. Por entre os diversos *shaders* disponíveis para
implementação, são destacados dois de interesse a este projeto:

1.  ***Vertex Shader***: Recebendo como *input* um vértice no espaço,
    este *shader* é usado no processamento de vértices. Tradicionalmente
    realiza operações matriciais MVP. É o único *shader* cuja presença é
    **obrigatória** num programa.

2.  ***Fragment Shader***: Recebendo um fragmento após o processo de
    rasterização como *input*, o *fragment shader* é capaz de o
    processar a fim de devolver uma determinada cor em conjunto com uma
    profundidade. Tradicionalmente, é neste *shader* que são aplicadas
    operações de iluminação e coloração.

Os *shaders* são compilados em *runtime* pelo *OpenGL*. Estes são
implementados numa linguagem própria, com sintaxe próxima à linguagem
`C`, denominada GLSL.

É possível enviar uma cópia de dados da CPU para a GPU com recurso a 
**variáveis uniformes** (*uniforms*). Estas funcionam como variáveis 
globais em qualquer *shader*. No entanto, dados relativos a vértices, 
normais e cores são passados por ***buffers***.

### *Vertex Attribute Object*

Com a finalidade de passar diversos dados para a GPU, é obrigatório
usar um gestor de atributos denominado por VAO. Este é geralmente usado 
para organizar atributos de diversos *buffers* (e.g. VBO e CBO).

Tendo em conta que o mesmo é mandatório na execução da *pipeline*, e não
havendo a necessidade de alocar *buffer objects*, é possível fornecer à
GPU um VAO pré-definido:

``` {.c++ linenos=""}
    unsigned int vaoHandle;
    glGenVertexArrays(1, &vaoHandle);
    glBindVertexArray(vaoHandle);
    glVertexAttrib1f(0, 0);
    glBindVertexArray(0);
```

# Tecnologias e Ferramentas

A implementação do projeto *CalcGL* envolveu a utilização de vários
recursos, documentados no presente Capítulo, em particular:

-   Linguagens de programação e respetivos *standards* ou versões
    eleitos;

-   Bibliotecas e *frameworks* para programar em *OpenGL*;

-   Código *open-source* adicional;

-   *Hardware* para a execução de testes.

## Tecnologias

Para a implementação do projeto *CalcGL* com *OpenGL* foi escolhida a
linguagem de programação C++, em particular o *standard* C++20 (indicado
ao compilador `g++` com a *flag* `-std=c++20`). A fim de melhorar a
experiência de utilização do *OpenGL*, foram utilizadas as seguintes
bibliotecas e *frameworks*:

-   **GLFW**: API simplificada para o *OpenGL*,
    igualmente multi-plataforma, permitindo a gestão de janelas,
    contextos, superfícies e comandos (rato, teclado e *joystick*);

-   **GLAD**: gerador
    automático de *loaders* para *OpenGL*;

-   **GLM**: biblioteca matemática baseada na linguagem dos 
    *shaders* do *OpenGL*, GLSL.

-   ***FreeType***: biblioteca de desenvolvimento dedicada à
    renderização de fontes em *bitmaps* utilizáveis, por exemplo, pelo
    *OpenGL*.

Há ainda a notar que a GPU é programada através de *shaders* com a
linguagem GLSL. Em particular, foi usada a versão *core*.

As versões destas bibliotecas e outro *software* auxiliar utilizado
estão sumariados na Tabela 3.1.


                            ***Software* / Tecnologia**                                  **Versão**
  ------------------------- ------------------------------------------------------------ --------------------------
  **Aplicação *OpenGL***                                                                 
                            *OpenGL*                                                     4.6
                            GLFW                                                         3.3.5
                            [GLAD]{acronym-label="GLAD" acronym-form="singular+abbrv"}   0.1.34
                            [GLM]{acronym-label="GLM" acronym-form="singular+abbrv"}     0.9.9.8
                            *FreeType*                                                   2.10.4
  **Relatório**                                                                          
                            XeTeX                                                        3.141592653-2.6-0.999993
                            *TeXstudio*                                                  4.2.2
  **Controlo de versões**                                                                
                            *git*                                                        2.36.1
                            *GitKraken*                                                  8.6.1

  : Ferramentas e tecnologias utilizadas, organizadas por categoria.
:::

## Código *Open Source*

Além das bibliotecas e *frameworks* referidas na Secção
3.1, código *open source* adicional foi
utilizado para facilitar a implementação de componentes que não fazem
parte do objetivo de estudo do projeto. Estes são:

-   ***CParse***: biblioteca para *parsing* de uma sequência
    de caracteres como uma expressão usando o algoritmo *Shunting-yard*
    de Dijkstra;

-   ***Learn OpenGL***: coleção de códigos de exemplo
    para o propósito de ensino de *OpenGL* moderno.

-   ***osdialog***: biblioteca multi-plataforma para acesso de caixas de
    diálogos do sistema operativo.

## *Hardware* e *Software* adicional

O *software* final foi testado em dois computadores distintos, listados
na Tabela 3.2. Os resultados destes testes serão abordados
em detalhe no Capítulo 5.

Para o desenvolvimento do projeto, foi utilizado o IDE ***Visual Studio Community 2022***. Os testes foram realizados no sistema operativo *Windows 11*. O *software* auxiliar 
para a gestão do código e escrita do presente relatório está listado na Tabela 3.1.

::: {#tab::hardware}
  -------------------------- -------------------------------------------------------------------------- ---------------------------------
  **Computador portátil**                                                                               
                             Processador (CPU)     Intel i5-8300H (2.3--4.0GHz)
                             Placa gráfica (GPU)   NVidia GTX 1050 Ti (4GB)
                             Memória RAM           16GB (DDR4 2133MHz)
  **Computador *desktop***                                                                              
                             Processador (CPU)     AMD Ryzen 7 2700X (3.7--4.3GHz)
                             Placa gráfica (GPU)   NVidia RTX 3070 (8GB)
                             Memória RAM           32GB (DDR4 3200MHz)
  -------------------------- -------------------------------------------------------------------------- ---------------------------------

  : Lista de *hardware* onde o projeto *CalcGL* foi testado.
:::

# Implementação {#ch::impl}

A implementação do projeto *CalcGL* será detalhada no presente Capítulo,
com particular foco nos seguintes pontos:

-   Requisitos funcionais do projeto;

-   Estrutura do código e o fluxo do mesmo;

-   Detalhes de implementação e desafios encontrados.

## Funcionalidades e Requisitos {#sec::impl:requisitos}

O projeto *CalcGL* segue uma lista de **funcionalidades** a implementar
baseados nos objetivos do projeto (Secção 1.3), em específico:

-   Renderização de funções implícitas com uso de *ray marching*;

-   Suporte a dois motores de cálculo:

    1.  Por *software* (i.e. os cálculos são efetuados pela CPU);

    2.  Aceleração por *hardware* (com uso da GPU);

-   Suporte a ficheiros externos contendo as funções implícitas a
    renderizar.

Foram ainda considerados os seguintes **requisitos** adicionais:

-   Abrir qualquer ficheiro `.function` dinamicamente por um meio
    gráfico (GUI);

-   Capacidade de personalizar a cor da superfície implícita;

-   Apresentar as fps alcançadas pelo programa para análise
    dos resultados.

## Lógica e Estruturação

A aplicação é composta por uma coletânea de classes, as quais comunicam
entre si de forma a gerir a representação das funções implícitas em
memória. O fluxo da aplicação é representado pela Figura 4.1.

A classe principal do programa é a **SISM**. Esta é encarregue pelo ciclo de
renderização pela gestão dos *shaders*, carregados aquando do arranque
do programa pelo *Shader Loader*.

![Representação esquemática do fluxo da *CalcGL*.](fluxo)

### Parâmetros de arranque {#ssec::impl:estrutura:start}

O programa disponibiliza uma coleção de argumentos que podem ser
passados pela linha de comandos (ou terminal) para alterar o seu
comportamento:

-   `--width` ou `-W`:\
    Modifica a largura da janela de renderização, em pixeis.\
    *Default*: 600.\
    Exemplo: `-W 1000` inicia o programa com uma largura de 1000 pixeis.

-   `--height` ou `-H`:\
    Modifica a altura da janela de renderização, em pixeis.\
    *Default*: 600.\
    Exemplo: `-H 1000` inicia o programa com uma altura de 1000 pixeis.

-   `--render` ou `-r`:\
    Define qual o motor de renderização a que o programa recorre.\
    Existem três modos implementados:

    -   `CPU`: motor de renderização por *software* com uso do algoritmo
        naïve *ray marching*;

    -   `GPU`: motor de renderização por aceleração de *hardware* com
        uso do algoritmo naïve *ray marching*;

    -   `SPHERES`: permite ao motor de renderização usar uma
        demonstração do algoritmo *sphere tracing*;

    *Default*: `GPU`.

-   `--threads` ou `-t`:
    Indica o número de *threads* que devem ser usadas pelo motor de
    renderização por *software*.
    *Default*: metade do número de núcleos lógicos disponibilizados pela
    CPU. Só é considerada caso o modo de renderização seja `CPU`. Se o 
    número especificado for superior ao *default*, o utilizador é alertado 
    para a possibilidade de problemas de *performance* no seu sistema.
    Exemplo: `-t 6` inicia o programa com seis *threads* prontas a serem
    usadas.

Está ainda disponível o comando de ajuda descrito como o argumento
`--help` ou `-h`, nesta situação o programa emite a informação
descrevendo os argumentos disponíveis e em seguida este sai do programa.
O processamento destes argumentos define o estado dos seguintes
atributos da SISM:

-   Motor de renderização;

-   Resolução da janela.

### *Shader Manager*

Este passo de arranque do programa só é feito caso a inicialização das
bibliotecas/*frameworks* GLFW e GLAD seja corretamente efetuada.

Os *shaders* são carregados pelo *Shader Manager*, o qual é instanciado
para cada um dos cinco programas (num total de dez *shaders* a compilar,
cada um com o seu respetivo *vertex* e *fragment shaders*):

1.  Motor de renderização por CPU;

2.  Motor de renderização acelerado por *hardware* (GPU);

3.  Motor de teste de *sphere tracing* (por GPU);

4.  Logótipo;

5.  Textos.

Anteriormente à compilação dos mesmos é efetuada uma **verificação
automática** dos *shaders*. Caso algum esteja em falta, o programa é
capaz de criar o respetivo *shader* predefinido e procede à sua
compilação. Assim que todos os *shaders* sejam corretamente compilados,
é gerada uma janela com a dimensão definida pelo atributo do
SISM (Figura 4.2). É esperado em caso de sucesso o seguinte *output*:

```
    Lauching CalcGL...
        Setting global variables... [OK]
        Setting relevant directories... [OK]
        Initializing GLFW and GLAD... [OK]
        Auto-checking shaders... (corrected 0 shaders) [OK]
        Loading shaders... [OK]
        Loading text renderer... [OK]
        Loading logo... [OK]
    Done.
```

![Ecrã inicial da aplicação *CalcGL*.](home)

## Motores de renderização

Dos três modos de renderização disponibilizados, dois são motores
propriamente ditos (por *software* e acelerado por *hardware*) e o
terceiro é um modo de demonstração de *sphere tracing*. Nos dois
primeiros, as funções são obtidas por ficheiros externos com a extensão
`*.function`. É obrigatório que as funções estejam escritas segundo a
sintaxe da linguagem GLSL.

### Cálculo por *Software*

Este motor de renderização pode executar em várias *threads*, onde cada
uma processa um conjunto de linhas contíguas do plano de visualização.
As funções são processadas pela biblioteca *CParse* e o algoritmo de
*ray marching* utilizado é o **naïve com *bisection***.

### Aceleração por *hardware*

O motor de renderização acelerado por *hardware* faz a **injeção de
funções** no *fragment shader* utilizado por este modo. Uma vez que o
*Shader Manager* pode compilar programas a qualquer momento, as funções
lidas do ficheiro `*.function` são injetadas num local predeterminado do
*fragment shader* padrão e um novo programa é compilado com este novo
*shader*, sendo o anterior descartado.

A parte do *fragment shader* modificado é o seguinte:

``` {.glsl linenos=""}
// Defines a implicit function
float evalImplicitFunc(vec3 point) {
    float x = point.x;
    float y = point.y;
    float z = point.z;
    float prod = 1.f;

    // <gamma conditions>

    return prod;
}
```

As funções lidas são multiplicadas a fim de formar uma Superfície $\Pi$
com um fator de *blending* $r = 0$. A sua injeção é feita no local do
comentário indicado por `<gamma conditions>`. Por exemplo, caso o
ficheiro defina três esferas:

$$\begin{aligned}
	x^2 + y^2 + z^2 - 1 = 0 \\
	(x-2)^2 + (y-1)^2 + z^2 - 1 = 0 \\
	(x-1)^2 + (y-2)^2 + (z-2)^2 - 1 = 0
\end{aligned}$$

O *fragment shader* será recompilado com as seguintes linhas de código
no lugar de `<gamma conditions>`.

``` {.glsl linenos="" startFrom="8"}
prod *= pow(x, 2.) + pow(y, 2.) + pow(z, 2.) - 1.;
prod *= pow(x-2., 2.) + pow(y-1., 2.) + pow(z, 2.) - 1.;
prod *= pow(x-1., 2.) + pow(y-2., 2.) + pow(z-2., 2.) - 1.;
```

Este modo **não implementa *bisection*** devido a uma limitação ao nível
da GPU.

Sendo unidades de cálculo muito especializadas, as
GPUs não contemplam instruções de *interrupt*, cuja consequência é a impossibilidade de parar os programas por si executados (conjunto de 
*shaders*) uma vez iniciados. Os *shaders* podem entrar em ciclos 
infinitos, os quais levariam ao total bloqueio de todo o sistema operativo 
por indisponibilidade da GPU. Desta forma, o sistema operativo
monitoriza o tempo de cada execução da GPU e, caso ultrapasse um determinado
*threshold* por si definido, é-lhe feito um *reset*.

Por este motivo, a implementação de *bisection* levou a este fenómeno de
*reset* da GPU, pelo que foi excluído do algoritmo final implementado no 
*fragment shader*.

Há ainda a notar que este motor não verifica as funções fornecidas nos
ficheiros `*.function` uma vez que não se enquadrava no objetivo
principal do projeto.

### Demonstração de *Sphere Tracing*

O modo de demonstração de *sphere tracing* distingue-se dos demais uma
vez que implementa diretamente no seu *fragment shader* as superfícies a
serem renderizadas, estando então as respetivas SDFs presentes diretamente 
no código. Outras funções implícitas não podem ser renderizadas, servindo 
então para mero efeito ilustrativo deste algoritmo de *ray marching*.

# Testes e Resultados

Durante a implementação e após a conclusão da versão final do projeto
*CalcGL*, foram realizados diversos testes a fim de analisar a exatidão
do algoritmo e a qualidade das renderizações obtidas. O presente
Capítulo explora os testes e respetivos resultados obtidos no sistema de
renderização com aceleração por *hardware*. Em último será feita uma
menção à renderização por *software*.

Doravante, referir-se-á ao programa final exclusivamente pelo seu nome,
*CalcGL*. Deve ainda ser assumido que, em toda a sequência de testes
feita e registada, a aplicação foi compilada em **modo *debug*** (no
qual o programa retorna informação detalhada útil neste processo).

## Aceleração por *hardware*

### Funções Implícitas Conhecidas

Conforme explorado na Secção 4.3, a fase de desenvolvimento para o sistema
de aceleração por *hardware* englobou duas fases distintas:

1.  Declaração das funções implícitas em código GLSL de forma direta no 
    *fragment shader*;

2.  Injeção das funções implícitas lidas a partir de ficheiros externos.

A primeira fase foi fundamental para limar a implementação do algoritmo
e fazer prova de conceito do uso exclusivo do *fragment shader*. A
primeira renderização feita com sucesso foi uma esfera colorida a verde
e com sombra (Figura 5.1), numa tentativa de replicar o resultado
obtido no *website* [shadertoy.com](shadertoy.com) (Figura 2.5) com 
recurso ao **algoritmo naïve**.

![Esfera renderizada no *CalcGL*, em fase inicial de testes, com o algoritmo naïve.](calcglsphere)

O *fragment shader* que produziu este resultado foi então adaptado para
mostrar 10 sólidos cujas SDFs são bem conhecidas, em conjunto com um plano
e dois pontos de luz para conferir o devido funcionamento das sombras.
Uma vez que as SDFs são conhecidas, foi utilizado o algoritmo de 
***sphere tracing***. O resultado obtido consta da Figura 5.2.

Por fim, o **parâmetro de suavização** $s$ (equação
([\[eq::suavizacao\]](#eq::suavizacao))) foi testado em função do tempo $t$ 
tal que $s = \cos(t)$; na Figura [5.3](#fig::spheresmooth) é apresentada uma 
*frame* do resultado obtido.

De notar que, em todos os testes com o algoritmo *sphere tracing*, o
programa executou à taxa de frequência do monitor (60 ou 75 fps).

![Renderização de 10 objetos no *CalcGL* usando o algoritmo de *spheretracing*](sphereoriginalblue)

![Renderização de 10 objetos no *CalcGL* usando o algoritmo de *spheretracing* com um fator de suavização $s$ dependente do tempo de execução $t$, em particular $s = \cos(t)$.](spheresmooth)

### Funções Implícitas Arbitrárias

Após o funcionamento de ambos os algoritmos naïve e *sphere tracing* ter
sido demonstrado com sucesso, procedeu-se aos testes finais a fim de
alcançar o objetivo principal do projeto. Para tal, foram utilizadas as
seguintes condições:

-   **Algoritmo**: naïve (uma vez que as SDFs não são conhecidas);

-   **Funções implícitas**: fornecidas por ficheiros externos;

-   ***Fragment shader***: recompilado em *runtime* para que as funções
    implícitas lhe sejam injetadas.

Dos testes realizados, três funções implícitas em particular são
apresentadas:

1.  **Superfície $\Pi$** (Figura 5.4), dada pela expressão:
    $$\begin{aligned}
    		f(x,y,z) = ~& \left(x^2 + y^2 + z^2 - 1\right) \times \\
    	                & \left((x-2)^2 + (y-1)^2 + z^2 - 1\right) \times \\
    		            & \left((x-1)^2 + (y-2)^2 + (z-2)^2 - 1\right) - 5
    	
    \end{aligned}$$

2.  ***Genus*** (Figura 5.5), dado pela expressão:
    $$f(x,y,z) = 2y(y^2 - 3x^2)(1-z^2) + (x^2 + y^2)^2 - (9z^2 - 1)(1 - z^2)$$

3.  **Fractal** (Figura 5.6), dado pela expressão:
    $$\begin{aligned}
    		f(x,y,z) =~ & (x^3 - 3xy^2 - 3xz^2)^2 + (y^3 - 3x^2y - 3yz^2)^2 + \\
    		            & (z^3 - 3x^2z + y^2z)^2  - (x^2 + y^2 + z^2)^3
    	
    \end{aligned}$$

Pode-se constatar que o algoritmo implementado perde bastante detalhe
com um fractal devido às limitações impostas a nível de cálculo. Uma
maior precisão seria necessária, mas tal levaria a uma execução bastante
pobre por parte da aplicação com o algoritmo naïve (conforme discutido
na Secção 4.3.2).

![Superfície $\Pi$ renderizada no *CalcGL* com algoritmo naïve.](calcglpisurf)

![*Genus* renderizado no *CalcGL* com algoritmo naïve.](calcglgenus)

![Fractal renderizado no *CalcGL* com algoritmo naïve sem *bisection*,
revelando a sua limitação para este
fim.](calcglfractal){#fig::calcglfractal width=".65\\textwidth"}

## Renderização por *Software* {#sec::testes:software}

A renderização por *software* (i.e. com recurso exclusivo à
[CPU]{acronym-label="CPU" acronym-form="singular+short"}) revelou-se
extremamente ineficiente, sendo impraticável. Não obstante, a
paralelização em *threads* foi testada a fim de verificar os tempos de
execução para uma única *frame*. O tempo foi contabilizado pela própria
aplicação.

Não só devido ao tempo de execução, mas também por exigir bastante da
[CPU]{acronym-label="CPU" acronym-form="singular+short"}, apenas foram
testados quatro cenários no computador *desktop* da Tabela
[3.2](#tab::hardware){reference-type="ref" reference="tab::hardware"}. O
tempo mínimo de execução foi alcançado com 2 *threads*, num tempo
aproximado de 1 hora e 40 minutos. Um maior número de *threads* revelou
ser prejudicial à *performance*. Estes tempos estão sumariados na Tabela
[5.1](#tab::render_cpu){reference-type="ref"
reference="tab::render_cpu"} e no gráfico da Figura
[5.7](#fig::cpurenderchart){reference-type="ref"
reference="fig::cpurenderchart"}.

::: {#tab::render_cpu}
    **Threads**   **Tempo** 
  ------------- ----------- -----------
                   Segundos      h:m's"
              1        7087   01:58'07"
              2        5984   01:39'44"
              4        7566   02:06'06"
              6       10414   02:53'34"

  : Tempo para a renderização de uma única *frame* com recurso à
  [CPU]{acronym-label="CPU" acronym-form="singular+abbrv"} para
  diferentes números de *threads*. O tempo em segundos foi arredondado à
  unidade.
:::

![Gráfico representativo dos tempos de execução em CPU, correlacionando o número de *threads* com o tempo em segundos para concluir uma *frame*.](cpurenderchart)

# Conclusões e Trabalho Futuro {#ch::conc}

## Conclusões {#sec::conc:conc}

Com o desenvolvimento e teste deste projeto foi possível:

-   Estudar as funções implícitas e o método de cálculo das respetivas
    iso-superfícies;

-   Analisar o modelo de renderização por volume;

-   Estudar o algoritmo da área de CG para renderização de superfícies
    implícitas por *ray marching* assim como *sphere tracing*;

-   Aprofundar o conhecimento na linguagem GLSL;

-   Conhecer em maior profundidade o funcionamento do *OpenGL* moderno.

Desta forma foi alcançado o objetivo principal: desenvolver e
implementar um sistema de visualização de funções implícitas com recurso
ao algoritmo de *ray marching* pelo *OpenGL* moderno.

Em relação ao motor de cálculo por *software*, a utilização do mesmo
revela não ser adequado para uso prático tendo em conta os longos tempos
de execução necessários para cálculo de uma única *frame*.

O algoritmo naïve de *ray marching* revelou-se adequado para a
renderização de qualquer função implícita com excepção dos fractais.
Estes necessitam de um nível de precisão que excede as capacidades do
*hardware* testado. Seria necessário conhecer a SDF específica de
cada fractal a ser renderizado a fim de se poder usar *sphere tracing*
e, assim, renderizá-los.

Por seu turno, o algoritmo de *sphere tracing*, apesar de apresentar
melhorias de *performance* consideráveis face ao algoritmo naïve de *ray
marching*, tem um uso mais limitado no contexto de funções implícitas
devido à inexistência de um método generalizado para cálculo da
SDF de uma superfície implícita arbitrária. É, contudo, o algoritmo 
preferencial para superfícies implícitas bem conhecidas.

## Trabalho Futuro

Ainda que, no contexto do projeto proposto, tenham sido cumpridos os
objetivos definidos, a conclusão deste revelou a existência de
oportunidades de melhoria e estudo nas áreas envolvidas, nomeadamente o
recurso a outros algoritmos, assim como o uso de *compute shaders* para
melhorar a *performance* do programa. Por outro lado, a optimização do
algoritmo naïve poderá ter um impacto positivo na renderização de
fractais com este algoritmo.

Seria igualmente relevante explorar a área matemática intrínseca ao
cálculo de uma SDF, com o propósito de alcançar um método generalizado 
para qualquer função implícita.
