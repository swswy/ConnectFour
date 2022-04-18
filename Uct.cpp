# include "Uct.h"


int Node::no_X = -2;
int Node::no_Y = -2;
int Node::len_M = 0;
int Node::len_N = 0;

//构造函数，每个节点中存储当前棋盘状态，落点，棋手身份以及模拟次数等信息
Node::Node(int **_board, const int *_top,  int _x, int _y, int _player)
    : winsNum(0), visitedNum(1), x(_x), y(_y), player(_player),
      parent(nullptr), topIndex(0), childrenNum(0), status(-1){

	memset(top, 0, K*sizeof(int));
	memset(state, 0, K*K*sizeof(int));
    for (int i = 0; i < len_N; i++) top[i] = _top[i];
    for (int i = 0; i < len_M; i++) {
        for (int j = 0; j < len_N; j++)
            state[i][j] = _board[i][j];
    }
}

Node::Node(int _board[][K], const int *_top, int _x, int _y, int _player, Node* _parent)
    :  winsNum(0), visitedNum(1), x(_x), y(_y), player(_player), 
      parent(_parent), topIndex(0), childrenNum(0), status(-1){

	memcpy(top, _top, sizeof(int)*K);
	top[y] = x;
	//如若遇到不可落子点，更改顶端状态数组
    if (x - 1 == no_X && y == no_Y) top[y] = x - 1 > 0? x - 1 : 0;
	memcpy(state, _board, K*K*sizeof(int));
	//落子
    if (x >= 0 && x < len_M && y >= 0 && y < len_N)
        state[x][y] = player;
}

Node::~Node() {
    for (int i = 0; i < childrenNum; i++)
        if (children[i]) 
			delete children[i];
}

//判断节点是否终止，status初始值为-1，status=0表示非终止状态
//status=1表示为获胜节点，表示一方已经获胜，而=3表示为平局状态
bool Node::isTerminal() {
    if (status != -1) return status;
    if (x < 0 || x >= len_M) return false;
    if (y < 0 || y >= len_N) return false;
    if (player == USER && 
        userWin(x, y, len_M, len_N, state))
        status = 1;
    else if (player == MACHINE && 
        machineWin(x, y, len_M, len_N, state))
        status = 1;
    else if (isTie(len_N, top))
        status = 3;
    else 
		status = 0;
    return status & 1;
}

//依据胜率等策略，选出最佳的子节点
Node* Node::bestChild(){
    Node *bestChild = nullptr;
    double maxProfit = (double)(0 - 0xfffffff);

    for (int i = 0; i < childrenNum; i++) {
		//寻找“好的与坏的”落子点
		if (children[i]->findGoodSpot())
			return children[i];
		int er = children[i]->findBadSpot();
		if (er == -1) return children[i];

        double a = (double)children[i]->winsNum / (double)children[i]->visitedNum;
        double b = C1 * sqrt(2 * log(this->visitedNum + 1.0) / children[i]->visitedNum);
        double c = (double)er / (double)log(children[i]->visitedNum + 1.0) * C2;
		//double c = (double)er / (double)sqrt(children[i]->visitedNum + 1.0) * C2;
        double tProfit = a + b + c;
        if (tProfit > maxProfit||(tProfit == maxProfit && rand() % 2)) {
			bestChild = children[i];
			maxProfit = tProfit;
        }
    }
    return bestChild;
}

//向上回溯，更新父节点的值
void Node::backup(int delta){
    Node *p = this;
    do {
        p->winsNum += delta;
        p->visitedNum += 1;
        delta = 0 - delta;
        if (abs(delta) > 1) {
            int absDelta = abs(delta) / 10;
            if (absDelta < 1) absDelta = 1;
            delta = delta > 0? absDelta: 0 - absDelta;
        }
        p = p->parent;
    } while (p);
}
//从当前结点扩展出子节点；返回nullptr表示已不可扩展
Node* Node::expand(){
    Node *newChild = nullptr;
	//通过节点成员变量topIndex来得出子节点
    while (top[topIndex] == 0 && topIndex < len_N) topIndex++;    
    if (topIndex >= len_N) return newChild;
	newChild = new Node(state, top, top[topIndex] - 1, topIndex, 3 - player,this);
	topIndex++;
    children[childrenNum++] = newChild;
    return newChild;
}

//从当前节点开始，随机落子直至决出胜负
int Node::defaultPolicy() {
	//增大决出胜负的节点权重
    if (isTerminal()) {
        if (status == 1) return KEY_PROFIT;
        else return 0;
    }
    int s[K][K];    
	int t[K];
	//拷贝棋盘及状态
	memcpy(s,state,sizeof(int)*K*K);
	memcpy(t, top, sizeof(int)*K);
    int _x = x; int _y = y;
    int identity = player; int delta = 0;
    //随机落子，直至决出胜负
    while (true) {
		identity = 3 - identity;
        do { _y = rand() % len_N; } while (t[_y] == 0);
        _x = t[_y] - 1; 
        s[_x][_y] = identity;
        t[_y] = _x;
        if (_x > 0 && _x - 1 == no_X && _y == no_Y) t[_y]--;
        if (identity == USER && userWin(_x, _y, len_M, len_N, s)) {
            delta = (player == USER)? 1: -1;
            break;
        } else if (identity == MACHINE && machineWin(_x, _y, len_M, len_N, s)) {
            delta = (player == MACHINE)? 1: -1;
            break;
        } else if (isTie(len_N, t)) {
            delta = 0; 
			break;
        }
    }
    return delta;
}


void Node::printState() {
	for (int i = 0; i < len_M; i++) {
		for (int j = 0; j < len_N; j++)
			cerr << state[i][j] << " ";
		cerr << "\n";
	}
}

int Node::findBadSpot() {
	if (x < 0 || x >= len_M ||
		y < 0 || y >= len_N) return 0;
	int aroundCnt = 0;
	// 检测竖向
	int count = 0;
	for (int i = x + 1; i < len_M; i++) {
		if (state[i][y] == 3 - player) count++;
		else break;
	}
	aroundCnt += count;
	if (count >= 3) return -1;

	// 检测横向
	count = 0;
	for (int i = y - 1; i >= 0; i--) {
		if (state[x][i] == 3 - player) count++;
		else break;
	}
	for (int i = y + 1; i < len_N; i++) {
		if (state[x][i] == 3 - player) count++;
		else break;
	}
	aroundCnt += count;
	if (count >= 3) return -1;

	int i, j;
	// 检测左上至右下
	count = 0;
	for (i = x - 1, j = y - 1; i >= 0 && j >= 0; i--, j--) {
		if (state[i][j] == 3 - player) count++;
		else break;
	}
	for (i = x + 1, j = y + 1; i < len_M && j < len_N; i++, j++) {
		if (state[i][j] == 3 - player) count++;
		else break;
	}
	aroundCnt += count;
	if (count >= 3) return -1;

	// 检测右上至左下
	count = 0;
	for (i = x + 1, j = y - 1; i < len_M && j >= 0; i++, j--) {
		if (state[i][j] == 3 - player) count++;
		else break;
	}
	for (i = x - 1, j = y + 1; i >= 0 && j < len_N; i--, j++) {
		if (state[i][j] == 3 - player) count++;
		else break;
	}
	aroundCnt += count;
	if (count >= 3) return -1;

	return aroundCnt;
}

int Node::findGoodSpot() {
	if (x < 0 || x >= len_M ||
		y < 0 || y >= len_N) return 0;
	// 检测竖向
	int count = 0;
	for (int i = x + 1; i < len_M; i++) {
		if (state[i][y] == player) count++;
		else break;
	}
	if (count >= 3) return 1;

	// 检测横向
	count = 0;
	for (int i = y - 1; i >= 0; i--) {
		if (state[x][i] == player) count++;
		else break;
	}
	for (int i = y + 1; i < len_N; i++) {
		if (state[x][i] == player) count++;
		else break;
	}
	if (count >= 3) return 1;

	int i, j;
	// 检测左上至右下
	count = 0;
	for (i = x - 1, j = y - 1; i >= 0 && j >= 0; i--, j--) {
		if (state[i][j] == player) count++;
		else break;
	}
	for (i = x + 1, j = y + 1; i < len_M && j < len_N; i++, j++) {
		if (state[i][j] == player) count++;
		else break;
	}
	if (count >= 3) return 1;

	// 检测右上至左下
	count = 0;
	for (i = x + 1, j = y - 1; i < len_M && j >= 0; i++, j--) {
		if (state[i][j] == player) count++;
		else break;
	}
	for (i = x - 1, j = y + 1; i >= 0 && j < len_N; i--, j++) {
		if (state[i][j] == player) count++;
		else break;
	}
	if (count >= 3) return 1;

	return 0;
}

// --------------------------------------------------------------- //

Uct::Uct(int **board, const int *top, const int M, const int N,int noX,int noY, int lastX, int lastY, int player){
		Node::no_X = noX;
		Node::no_Y = noY;
		Node::len_M = M;
		Node::len_N = N;
		root = new Node(board,top,lastX,lastY, player);
}

Uct::~Uct() {
    if (root) delete root;
}
//选择要模拟的节点
Node* Uct::treePolicy() {
    Node *p = root;
    while(!p->isTerminal()) {
        Node *q = p->expand();
        if (q) {
            return q;
        } else {
            p = p->bestChild();
        }
    }
    return p;
}
//uct搜索
Node* Uct::uctSearch() {
    clock_t startTime = clock();
    int cnt = 0;
    while ((double)(clock() - startTime) / (double)CLOCKS_PER_SEC < TIME_LIMIT) {
        cnt++;
        Node *v = treePolicy();			//选择或扩展节点
        int delta = v->defaultPolicy();	//模拟计算收益
        v->backup(delta);				//回溯
    }
	//printf("the cnt is %d\n", cnt);
    Node *bestChild = root->bestChild();
    return bestChild;
}