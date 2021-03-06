#include "adt_huffman.h"
#include "headers.h"

TREE *create_node(BYTE element, int frequency)
{
	BYTE *item_aux = malloc(sizeof(BYTE));
	*item_aux = element;

	TREE *new_node = malloc(sizeof(TREE));
	new_node->frequency = frequency;

	new_node->left = NULL;
	new_node->right = NULL;
	new_node->item = item_aux;

	return new_node;
}

// THIS FUNCTION RETURNS THE ITEM OF THE NODE
BYTE get_node_item(TREE *node)
{
	BYTE *item = (BYTE*) node->item;
	return *item;
}

// THIS FUNCTION CREATES A HEAP
HEAP * create_heap()
{
	HEAP *newHeap = malloc(sizeof(HEAP));
	newHeap->size = 0;
	
	TREE **data = malloc(sizeof(TREE**) * 256);

	for (int i = 0; i < 256; i++)
		data[i] = NULL;

	newHeap->data = data;

	return newHeap;
}

int parentIndex(int i) {return ((i + 1) >> 1) - 1;}

int leftChildIndex(int i) {return ((i + 1) << 1) - 1;}

int rightChildIndex(int i) {return (i + 1) << 1;}

// THIS FUNCTION ENQUEUES A NODE IN A HEAP
void enqueue(HEAP *heap, TREE *node)
{
	int i, p;

	TREE **auxData = heap->data;
	i = heap->size;
	heap->size++;
	
	auxData[i] = node;
	p = parentIndex(i);

	while ((i != 0) && (auxData[i]->frequency <= auxData[p]->frequency))
	{
		TREE *aux;

		aux = auxData[i];
		auxData[i] = auxData[p];
		auxData[p] = aux;

		i = p;
		p = parentIndex(i);
	}

	heap->data = auxData;
}

void min_heapify(HEAP *heap, int i)
{
	int l, r;

	l = leftChildIndex(i);
	r = rightChildIndex(i);

	TREE **auxData = heap->data;

	if ((l < heap->size) && (auxData[l]->frequency <= auxData[i]->frequency))
	{
		TREE *aux;

		aux  = auxData[i];
		auxData[i] = auxData[l];
		auxData[l] = aux;

		heap->data = auxData;
		min_heapify(heap, l);
	}
	if ((r < heap->size) && (auxData[r]->frequency <= auxData[i]->frequency))
	{
		TREE *aux;

		aux  = auxData[i];
		auxData[i] = auxData[r];
		auxData[r] = aux;

		heap->data = auxData;
		min_heapify(heap, r);
	}
}

// THIS FUNCTION DEQUEUES A NODE FROM THE HEAP
TREE * dequeue(HEAP *heap)
{
	TREE **auxData = heap->data;
	TREE *aux = auxData[0];

	heap->size--;
	auxData[0] = auxData[heap->size];

	heap->data = auxData;
	min_heapify(heap, 0);
	
	return aux;
}

//THIS FUNCTION COUNTS THE FRENQUENCY OF THE BYTES IN THE FILE
void count_frequency(FILE *file, HASH *hash)
{
	BYTE element;
	//fseek(file, 0, SEEK_END);
	while(fscanf(file,"%c", &element) != EOF)
	{
		hash->array[element]->frequency++;
	}
}

void mount_heap(HEAP *heap, HASH *hash)
{
	TREE *aux;

	for (int i = 0; i < 256; i++)
	{
		if (hash->array[i]->frequency > 0)
		{
			aux = create_node(i, hash->array[i]->frequency);
			enqueue(heap, aux);
		}
	}
}

TREE * create_huffman_tree(HEAP *heap)
{
	TREE *l, *r, *newNode;

	if (heap->size == 1)
	{
		newNode = create_node('*', -1);
		newNode->left = dequeue(heap);
		return newNode;
	}

	while(heap->size > 1)
	{
		l = dequeue(heap);
		r = dequeue(heap);

		newNode = create_node('*', l->frequency + r->frequency);

		newNode->left = l;
		newNode->right = r;

		enqueue(heap, newNode);
	}

	return dequeue(heap);
}

HASH* create_hash()
{
    HASH* hash = malloc(sizeof(HASH));
    
    for (int i = 0; i < 256; i++)
    {
        hash->array[i] = malloc(sizeof(ELEMENT));
        
        hash->array[i]->frequency = 0;
        hash->array[i]->binary[0] = '\0';
    }
    return hash;
}

// THIS FUNCTION READS THE NEW BINARY VALUE
void binary_read(TREE *node, HASH* hash, char *string)
{
	if (node != NULL)
    {
        BYTE aux = get_node_item(node);

        if (node->left == NULL)
        {
			//printf("%c : %s\n",aux, string);
            strcat(hash->array[aux]->binary, string); 
        }
        else
        { 
            binary_read(node->left,  hash, strcat(string, "0"));

            int size = strlen(string);
            string[size - 1] = '\0';

            binary_read(node->right, hash, strcat(string, "1"));

            size = strlen(string);
            string[size - 1] = '\0';
        }
    }
}

// THIS FUNCTION SAVES THE TREE IN PRE-ORDER ON A STRING
void get_tree(TREE *huff, BYTE string[], int *len)
{
	if (huff != NULL)
	{
        BYTE aux;
        aux = get_node_item(huff);

		if(huff->left == NULL){

			if(aux == '*'  || aux == '\\')
			{
				string[*len] = '\\';
				*len+=1;
			}
			string[*len] = aux;
			*len += 1;
		}
		else{
			string[*len] = aux;
			*len += 1;
			get_tree(huff->left, string, len);
			get_tree(huff->right, string, len);
		}
	}

}

int is_bit_i_set(BYTE c, int i)
{
    BYTE mask = 1 << i;
    return mask & c;
}
BYTE set_bit(BYTE c, int i)
{
    BYTE mask = 1 << i;
    return mask | c;
}

int getTrashLength(HASH* hash)
{
    long long int trash = 0;
    for (int i = 0; i < 256; i++)
    {
        trash += strlen(hash->array[i]->binary) * hash->array[i]->frequency;
    }
    return (8 - (trash % 8)) % 8;
}

void write_header(FILE* output_file, HASH* hash, TREE* tree)
{
    int trash_size = getTrashLength(hash);
    BYTE binary[2] = {0};
    
    
    BYTE pre_order_tree[10000];

	int length = 0;
    get_tree(tree, pre_order_tree, &length);

    binary[0] = trash_size << 5 | length >> 8;
    binary[1] = length;

    fwrite(binary, 1, 2, output_file);

    fwrite(pre_order_tree, 1, length, output_file);
}

void write_new_binary(FILE *input_file,FILE *output_file , HASH *hash)
{
	BYTE element;
    BYTE new_byte = 0;
	int bit = 7;

	fseek(input_file, 0, SEEK_SET);

	while(fscanf(input_file,"%c", &element) != EOF)
	{
       for(int i = 0; hash->array[element]->binary[i] != '\0'; i++)
	   {
		   if(bit < 0)
		   {
			   fprintf(output_file,"%c",new_byte);
			   new_byte = 0;
			   bit = 7;
		   }
		   if(hash->array[element]->binary[i] == '1')
		   {
			   new_byte = set_bit(new_byte,bit);
		   }
		   bit--;
	   }
	}
	fprintf(output_file,"%c",new_byte);
}

void write_file(TREE* tree, HASH* hash, FILE *input_file, char input_file_name[])
{
    FILE* output_file;
    strcat(input_file_name, ".huff");

    output_file = fopen(input_file_name, "wb");

    write_header(output_file, hash, tree);

    write_new_binary(input_file,output_file,hash);

	fclose(output_file);
}

void compress_file(FILE *input_file, char *file_name)
{

	HEAP *heap = create_heap();
	TREE *huffman_tree = NULL;

	char string_binary[256];
    string_binary[0] = '\0';
	HASH* hash = create_hash();

	count_frequency(input_file, hash);

	mount_heap(heap, hash);

	huffman_tree = create_huffman_tree(heap);

    binary_read(huffman_tree, hash, string_binary);

    write_file(huffman_tree, hash, input_file, file_name);
	
}
