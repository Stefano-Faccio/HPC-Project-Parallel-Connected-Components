# Leggo da un file il numero di nodi e di archi
# poi leggo gli archi e li salvo in un vettore

import sys
import random
# function to find cumulative sum of list
from itertools import accumulate 

my_index = 0

def random_cc(n, m, label, iter):
    #Aumento il contatore delle iterazioni
    iter[0] += 1
    print("Iterazione: ", iter[0])
    
    #Caso base
    if n == 0 or len(m) == 0:
        return
    
    #Genero i numeri casuali
    coin_toss = []
    for i in range(n):
        coin_toss.append(random.randint(0, 1))
        #coin_toss.append(my_random())
    #Attacco i nodi
    for (u, v) in m:
        if coin_toss[u] == 0 and coin_toss[v] == 1:
            label[u] = label[v]
        if coin_toss[v] == 0 and coin_toss[u] == 1:
            label[v] = label[u]
            '''
        if (u == 0 and v == 7) or (u == 7 and v == 8):
            print ("U: ", u, " V: ", v)
            print("Label U: ", label[u], " Label V: ", label[v])
            print("Coin toss U: ", coin_toss[u], " Coin toss V: ", coin_toss[v])
            print()
            '''
            

    S = [0] * len(m)
    for i in range(len(m)):
        if label[m[i][0]] != label[m[i][1]]:
            S[i] = 1
        else:
            S[i] = 0
    
    S = list(accumulate(S)) 
    F = [0] * (S[-1])
        
    for i in range(len(m)):
        if label[m[i][0]] != label[m[i][1]]:
            F[S[i] - 1] = (label[m[i][0]], label[m[i][1]])
            
    #print("Coin toss: ", coin_toss)
    #print("Label: ", label)
    #print("F: ", F)
            
    #Chiamata ricorsiva
    random_cc(n, F, label, iter)
    
    #Mappo sul grafo originale
    for (u, v) in m:
        if v == label[u]:
            label[u] = label[v]
        if u == label[v]:
            label[v] = label[u]
            
    return
    
def my_random():
    array = [0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1]
    global my_index
    tmp = array[my_index]
    #print("Random: ", tmp)
    my_index = my_index + 1
    
    return tmp


def read_graph(filename):
    with open(filename, 'r') as f:
        n, m = map(int, f.readline().split())
        G = []
        for _ in range(m):
            a, b = map(int, f.readline().split())
            if a != b:
                G.append((a, b))
        print("Nodi: ", n)
        print("Archi: ", len(G))
        print("Scartati: ", m - len(G))
        
    return G, n


if len(sys.argv) != 2:
    print("Usage: python3 py.py <filename>")
    sys.exit(1)
filename = sys.argv[1]
edges, nodes = read_graph(filename)
labels = []
for i in range(nodes):
    labels.append(i)
iterations = [0]
random_cc(nodes, edges, labels, iterations)

#print("Etichette: ", labels)
print("Numero di componenti connesse: ", len(set(labels)))
print("Numero di iterazioni: ", iterations[0])

    