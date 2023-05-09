#include "rbtree.h"
#include <stdlib.h>
#include <assert.h>


typedef enum { RBTREE_LEFT, RBTREE_RIGHT } direct_t;

node_t* make_node(const key_t key){
  node_t* node = (node_t *)calloc(1,sizeof(node_t));
  node->color = RBTREE_RED;
  node->key = key;
  node->parent = NULL;
  node->left = NULL;
  node->right = NULL;
  return node;
}

void rotate(rbtree *t, node_t *x, direct_t dir){
  node_t* y;

  if(dir == RBTREE_LEFT) // left rotate
  {
    y=x->right;
    x->right = y->left;
    if(y->left != t->nil) y->left->parent = x;
  }
  else // right rotate
  {
    y = x->left;
    // x의 left에 y의 right를 연결한다.(y의 right가 y를 대체)
    x->left = y->right;
    if(y->right != t->nil) y->right->parent = x;
  }

  // y를 x의 부모와 연결시킨다.
  y->parent = x->parent;

  // 만약 x가 루트 노드인 경우
  if(x->parent == t->nil) t->root = y;
  else if(x==x->parent->left) x->parent->left = y;
  else x->parent->right = y;

  if(dir == RBTREE_LEFT) y->left = x;
  else y->right = x;
  x->parent = y;

}


// u를 v로 대체하는 작업(u의 부모와 v를 연결)
void transplant(rbtree *t, node_t *u, node_t *v){
  //u가 root 노드인 경우
  if(u->parent==t->nil) t->root = v;
  else if(u==u->parent->left) u->parent->left = v;
  else  u->parent->right = v;
  v->parent = u->parent;
}


// 여러 개의 tree를 생성할 수 있어야 하며 각각 다른 내용들을 저장할 수 있어야 합니다.
rbtree *new_rbtree(void) {
  rbtree *p = (rbtree *)calloc(1, sizeof(rbtree));
  
  // TODO: initialize struct if needed

  // RB tree에 nill node 생성
  p->nil = make_node(0);
  // RB tree의 node 색깔 변경
  p->nil->color = RBTREE_BLACK;
  // RB tree의 root를 일단 nil node와 연결
  p->root = p->nil;
  return p;
}


void delete_node(rbtree *t, node_t *n){
  if(n == t->nil) return;
  if(n->left!=t->nil) delete_node(t,n->left);
  if(n->right!=t->nil) delete_node(t,n->right);
  free(n);
}

// 해당 tree가 사용했던 메모리를 전부 반환해야 합니다. (valgrind로 나타나지 않아야 함)
void delete_rbtree(rbtree *t) {
  // TODO: reclaim the tree nodes's memory
  delete_node(t,t->root);
  free(t->nil);
  free(t);
}

node_t *insert_case1(node_t *y,node_t *z){
  z->parent->color = RBTREE_BLACK;
  // z의 조부모를 Red로 만들기 위해 y를 Black으로 바꾼다
  y->color = RBTREE_BLACK;
  // z의 조부모를 Red로 만든다.
  z->parent->parent->color = RBTREE_RED;
  // // z를 z의 조부모로 만든다.
  return z->parent->parent;
}

node_t *insert_case2(rbtree *t,node_t *z,direct_t dir){
  z=z->parent;
  rotate(t,z,dir);
  return z;
}

void insert_case3(rbtree *t, node_t *z,direct_t dir){
 // z의 부모를 Black으로 만듦으로서 속성 4 만족
  z->parent->color = RBTREE_BLACK;
  // z의 조부모의 자식 모두 Black이기 때문에 Red로 변경 가능
  z->parent->parent->color=RBTREE_RED;
  // z의 조부모를 기준으로 right_rotate.
  // z의 조부모가 z의 부모의 자식으로 들어가면서 balance가 맞춰짐
   if(dir==RBTREE_LEFT)rotate(t,z->parent->parent,RBTREE_RIGHT);
   else rotate(t,z->parent->parent,RBTREE_LEFT);
}

// insert를 위반한 경우
void insert_fixup(rbtree *t, node_t *z){
  node_t *y;
  //Red의 부모가 Red일 때까지 반복
  while(z->parent->color == RBTREE_RED){
    // z의 부모가 조부모의 left인 경우 -> left가 더 많다는 의미
    if(z->parent==z->parent->parent->left){
      // y는 삼촌 노드
      y=z->parent->parent->right;
      // 삼촌이 Red일 때 경우 1이다.
      if(y->color==RBTREE_RED) z = insert_case1(y,z);
      else{
        // z의 삼촌이 Black일 때 경우 2와 3
        // z가 부모의 right일 때 경우 2, 경우 2를 경우 3으로 변경해야 함
        if(z == z->parent->right) z = insert_case2(t,z,RBTREE_LEFT);
        // z가 left일 때 경우 3
        insert_case3(t,z,RBTREE_LEFT);
      }

    }else{
      // 위의 경우와 left와 right만 바꿔주면 된다.
      y=z->parent->parent->left;
      if(y->color==RBTREE_RED) z = insert_case1(y,z);
      else // z의 삼촌이 Black일 때 경우 2와 3
      {
        if(z == z->parent->left) z = insert_case2(t,z,RBTREE_RIGHT);
        insert_case3(t,z,RBTREE_RIGHT);
      }
    }
  }
  // root의 색은 항상 Black (속성2)
  t->root->color = RBTREE_BLACK;
}

// 구현하는 ADT가 multiset이므로 이미 같은 key의 값이 존재해도 하나 더 추가 합니다.
node_t *rbtree_insert(rbtree *t, const key_t key) {
  // TODO: implement insert
  // 들어온 key값을 담는 node 생성
  node_t *z = make_node(key);
  node_t* y = t->nil;
  node_t* x = t->root;

  while(x!=t->nil){
    y = x;
    if(z->key < x->key) x=x->left;
    else x = x->right;
  }

  z->parent = y;

  if (y == t->nil) t->root = z;
  else if(z->key < y->key) y->left = z;
  else y->right = z;
  
  z->left = t->nil;
  z->right = t->nil;
  z->color = RBTREE_RED;

  insert_fixup(t,z);

  return z;
}

// RB tree내에 해당 key가 있는지 탐색하여 있으면 해당 node pointer 반환
// 해당하는 node가 없으면 NULL 반환
node_t *rbtree_find(const rbtree *t, const key_t key) {
  // TODO: implement find
  node_t *n = t->root;

  while(n!=t->nil){
    if (key < n->key) n = n->left;
    else if(key>n->key) n = n->right;
    else return n;
    
  }

  return NULL;
}

// RB tree 중 최소 값을 가진 node pointer 반환
node_t *rbtree_min(const rbtree *t) {
  // TODO: implement find

  node_t* n = t->root;
  while(n->left!=t->nil) n=n->left;

  return n;
}

// 최대값을 가진 node pointer 반환
node_t *rbtree_max(const rbtree *t) {
  // TODO: implement find
  node_t* n = t->root;
  while(n->right!=t->nil) n=n->right;

  return n;
}

node_t *delete_case1(rbtree *t, node_t *w, node_t *x,direct_t dir){
  w->color = RBTREE_BLACK;
  x->parent->color = RBTREE_RED;
  if(dir == RBTREE_LEFT) rotate(t,x->parent,RBTREE_LEFT);
  else rotate(t,x->parent,RBTREE_RIGHT);

  if(dir == RBTREE_LEFT) return x->parent->right;
  return x->parent->left;
}

node_t *delete_case2(node_t *w, node_t *x){
  w->color = RBTREE_RED;
  return x->parent;
}

node_t *delete_case3(rbtree *t, node_t *w, node_t *x,direct_t dir){
  if(dir==RBTREE_LEFT) w->left->color = RBTREE_BLACK;
  else w->right->color = RBTREE_BLACK;
  
  w->color = RBTREE_RED;
  
  if(dir==RBTREE_LEFT) rotate(t,w,RBTREE_RIGHT);
  else rotate(t,w,RBTREE_LEFT);

  if(dir==RBTREE_LEFT) return x->parent->right;
  return x->parent->left;
}

void delete_case4(rbtree *t, node_t *w, node_t *x, direct_t dir){
  w->color = x->parent->color;
  x->parent->color = RBTREE_BLACK;
  
  if(dir==RBTREE_LEFT)
  {
    w->right->color = RBTREE_BLACK;
    rotate(t,x->parent,RBTREE_LEFT);
  }
  else{
    w->left->color = RBTREE_BLACK;
    rotate(t,x->parent,RBTREE_RIGHT);
  }

  
}


void delete_fixup(rbtree *t,node_t *x){
  node_t *w;
  while(x!=t->root && x->color==RBTREE_BLACK){
    if(x==x->parent->left){
      w = x->parent->right;
      //경우 1
      if(w->color == RBTREE_RED) w = delete_case1(t,w,x,RBTREE_LEFT);

      //경우 2
      if(w->left->color==RBTREE_BLACK && w->right->color == RBTREE_BLACK) x = delete_case2(w,x);
      else
      {
        //경우 3
        if(w->right->color == RBTREE_BLACK) w = delete_case3(t,w,x,RBTREE_LEFT);

        //경우 4
        delete_case4(t,w,x,RBTREE_LEFT);
        
        //x를 루트로 바꿈으로써 종료
        x = t->root;
      }
    }else{
      w = x->parent->left;
      //경우1
      if(w->color == RBTREE_RED) w = delete_case1(t,w,x,RBTREE_RIGHT);
      //경우2
      if(w->right->color==RBTREE_BLACK && w->left->color == RBTREE_BLACK) x = delete_case2(w,x);
      else
      {
        //경우 3
        if(w->left->color == RBTREE_BLACK) w = delete_case3(t,w,x,RBTREE_RIGHT);
        
        //경우4
        delete_case4(t,w,x,RBTREE_RIGHT);
        
        //x를 루트로 바꿈으로써 종료
        x = t->root;
      }
    }
  }
  x->color = RBTREE_BLACK;
}

// successor를 구하는 함수
node_t *minimum(rbtree *t,node_t *y){
  //y는 삭제할 z를 대체할 노드
  while(y->left!=t->nil) y=y->left;
  return y;
}



// RB tree 내부의 ptr로 지정된 node를 삭제하고 메모리 반환
int rbtree_erase(rbtree *t, node_t *z) {
  // TODO: implement erases
  node_t *x;
  node_t *y = z;
  // 삭제할 색깔을 저장
  color_t  y_origin_color = y->color;


  if(z->left == t->nil)
  {
    x = z->right;
    transplant(t,z,z->right);
  }
  else if(z->right == t->nil)
  {
    x = z->left;
    transplant(t,z,z->left);
  }
  else
  {
    // z에 와야하는 successor를 구함
    y = minimum(t,z->right);
    // successor의 색이 삭제 됨
    y_origin_color = y->color;
    
    x = y->right;
    
    // successor가 z의 오른쪽 자식인 경우
    if(y->parent == z) x->parent = y;
    else{
      //y를 y의 right로 대체한다.(이진트리이기 때문에)
      transplant(t,y,y->right);
      //y가 z를 대체해야 하므로 z의 오른쪽 자식이 y의 오른쪽 자식이 되게 한다.
      y->right = z->right;
      y->right->parent = y;
    }
    //z가 y로 대체된다.
    transplant(t,z,y);

    //z의 왼쪽 자식마저 y의 왼쪽 자식이 되면서 y가 완전히 z를 대체한다.
    y->left = z->left;
    y->left->parent = y;
    y->color = z->color;
  }
  // 삭제 된 색이 Black이라면 속성 2,4,5를 위반할 수 있으므로 고쳐야한다.
  if (y_origin_color == RBTREE_BLACK) delete_fixup(t,x);
  free(z);
  return 0;
}

void in_order(const rbtree *t,key_t *arr, node_t *n, int *idx){
  if(n->left!=t->nil) in_order(t,arr,n->left,idx);
  arr[(*idx)++] = n->key;
  if(n->right!=t->nil) in_order(t,arr,n->right,idx);

}

// RB tree의 내용을 key 순서대로 주어진 array로 변환
// array의 크기는 n으로 주어지며 tree의 크기가 n 보다 큰 경우에는 순서대로 n개 까지만 변환
// array의 메모리 공간은 이 함수를 부르는 쪽에서 준비하고 그 크기를 n으로 알려줍니다.
int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n) {
  // TODO: implement to_array
  if(t->root==NULL) return 0;
  int idx = 0;
  in_order(t,arr, t->root, &idx);
  return 0;
}

