# ifndef UCTREE_H_
# define UCTREE_H_

# define USER 1
# define MACHINE 2
# define TIME_LIMIT 2.66
# define C1 0.717
# define C2 0.1
# define KEY_PROFIT 10000

#include <math.h>
#include <random>
#include <iostream>
#include "Judge.h"
#include <ctime>
#include <cstring>
using std::cerr;

class Node {
public:
	static int no_X, no_Y;
	static int len_M, len_N;
	int state[K][K];		
	int top[K];			//���״̬
    int winsNum, visitedNum;
	int x, y;			//����λ��
    int player;			//�������
	Node *parent;
	Node *children[K];
    int topIndex;		//��չλ��
	int childrenNum;	//�ӽڵ�����
	int status;			//�ڵ�����״̬

	
    Node(int **_board, const int *_top, int _x, int _y, int _player);
    Node(int _board[][K], const int *_top, int x, int y, int _player, Node* _parent);
    ~Node();
    bool isTerminal();
    Node *bestChild();
    Node *expand();
    void backup(int delta);
    int defaultPolicy();

    int findBadSpot();
    int findGoodSpot();
    void printState();
};

class Uct {
public:
    Node *root;
    Node *treePolicy();
	
    Uct(int **board, const int *top, const int M, const int N, int noX, int noY, int lastX, int lastY, int player);
    ~Uct();
	Node* uctSearch();
};

# endif