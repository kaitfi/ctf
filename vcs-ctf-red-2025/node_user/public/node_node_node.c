#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>


typedef struct node
{
    size_t id, n;
    char data[0x10];
    void (*func_ptr)(char*);
    size_t link[0x10];
} node;

node *nodes[0x10];

void node_method(char words[])
{
    puts(words);
}

node* Create_Node(size_t id)
{
    node *tmp = (node*)malloc(sizeof(node));
    if(!tmp)
        return NULL;
    else
    {
        tmp->id = id;
        tmp->n = 0;
        tmp->func_ptr = &node_method;
        memset(tmp->data, 0, sizeof(tmp->data));
        memset(tmp->link, -1, sizeof(tmp->link));
        return tmp;
    }
}

void Link_Node(size_t src, size_t dest)
{
    if(nodes[src] && nodes[dest])
    {
        for(int i = 0; i < nodes[src]->n; i++)
            if(nodes[src]->link[i] == dest)
                exit(-1);

        node *src_node = nodes[src], *dest_node = nodes[dest];
        src_node->link[src_node->n++] = dest;
        dest_node->link[dest_node->n++] = src;
        src_node->func_ptr("Done!");
    }
    else
    {
        puts("Invalid");
    }
}

void Read_Graph(size_t start)
{
    size_t is_visited[0x10] = {0}, n;
    node *node_list[0x10], *tmp;

    if(!nodes[start])
    {
        puts("Not created node");
        return;
    }
    
    node_list[0] = nodes[start];
    
    n = 1;
    while(n)
    {
        tmp = node_list[0];
        printf("Node: %llu\n", tmp->id);

        is_visited[tmp->id] = 1;

        tmp->func_ptr("Data: ");
        read(0, tmp->data + strlen(tmp->data), 0x10);
        tmp->func_ptr("Done!");

        for(int i = 0; i < tmp->n; i++)
        {
            if(!is_visited[tmp->link[i]])
                node_list[n++] = nodes[tmp->link[i]];
        }

        //remove the first element in node_list
        for(int i = 0; i < n; i++)
            node_list[i] = node_list[i+1];
        n--;
    } 
    puts("Data saved");
}

void call_me()
{
    system("cat flag");
}

void menu()
{
    puts("1. Create node");
    puts("2. Link nodes");
    puts("3. Save data");
    puts("4. Exit");
    puts(">> ");
}

void setup()
{
    setvbuf(stdin, 0, 2, 0);
    setvbuf(stdout, 0, 2, 0);
    setvbuf(stderr, 0, 2, 0);
}

int main()
{
    size_t choice, id1, id2;
    setup();
    memset(nodes, 0, sizeof(nodes));
    while(1)
    {
        menu();
        scanf("%llu", &choice);
        switch (choice)
        {
        case 1:
            puts("Id: ");
            scanf("%llu", &id1);
            if(id1 < 0x10 && nodes[id1])
                puts("Node exists");
            else if(id1 >= 0x10)
                puts("Invalid id");
            else
            {
                nodes[id1] = Create_Node(id1);
                nodes[id1]->func_ptr("Created");
            }
            break;
        case 2:
            puts("Node 1: ");
            scanf("%llu", &id1);
            puts("Node 2:");
            scanf("%llu", &id2);
            if(id1 < 0x10 && id2 < 0x10 && id1 != id2)
                Link_Node(id1, id2);
            else
                puts("Invalid");
            break;
        case 3:
            puts("Read from: ");
            scanf("%llu", &id1);
            Read_Graph(id1);
            break;
        case 4:
            exit(0);
        default:
            puts("Invalid choice");
            break;
        }
    }
    return 0;
}
