## **Projeto da Cadeira INF 01047 - Fundamentos de Computação Gráfica (2016/01)**##
*Prof. Marcelo Walter*

**Objetivo:**
Desenvolver um jogo com a mecânica semelhante ao jogo Dig Dug II.
Utilizando tecnologia OpenGL alinhado aos conhecimento adquiridos no decorrer do semestre.



**Descrição Geral:**

No jogo manipulamos um personagem que pode se mover nas quatro direções tradicionais (cima, baixo, esquerda, direita) através dos controles WASD.

O personagem esta em um mapa finito, e caso atravesse o limite do mapa será caraterizada "derrota".

No mapa existem os elementos: 

* Esfera
* * Elemento 3D
* * Elemento de posição fixa, em caso de colisão o personagem não avança.
* Inimigo
* * Elemento 3D
* * Elemento de posição variável, em caso de colisão caracteriza "derrota".
* * O inimigo possui inteligencia para detectar que esta a menos de 4 casas de distancia do personagem e nesse caso, iniciar uma perseguição.
* Buraco
* * Textura modificada 
* * O personagem pode andar sobre, os inimigos não.
* * Com o personagem em cima de um buraco será possível criar uma rachadura na direção para qual a visão do personagem aponta.
* Rachadura
* * Textura modificada 
* * O personagem pode andar sobre, os inimigos não.
* * Quando as rachaduras fecharem um "poligono" o menor poligono será "desmoronado".




![Print1.jpg](https://bitbucket.org/repo/RLxz7d/images/2667821189-Print1.jpg)
![Print2.png](https://bitbucket.org/repo/RLxz7d/images/642582165-Print2.png)
![Print3.jpg](https://bitbucket.org/repo/RLxz7d/images/3466234394-Print3.jpg)