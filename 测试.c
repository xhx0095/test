 #include "mpi.h"
 #include <stdio.h>

 #define ROWS 100
 #define COLS 100
#define min(x, y) ((x) > (y) ? (y) : (x))
 int main(int argc, char *argv[])
 {
	     int rows = 10, cols = 10;
	     int master = 0;
	     int myid, numprocs;
	     int i, j;
	     float a[ROWS][COLS], b[COLS], c[COLS];
	     float row_result;
	     MPI_Status status;
	
		     MPI_Init(&argc, &argv);
	     MPI_Comm_rank(MPI_COMM_WORLD, &myid);
	     MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	
		     /*master进程*/
		     if (master == myid) {
		         /*初始化矩阵a和b*/
			         for (j = 0; j < cols; j++) b[j] = 1;
		         for (i = 0; i < rows; i++)
			         {
			             for (j = 0; j < cols; j++)
				             {
				                 a[i][j] = i;
				             }
			         }
		         /*只在master进程中初始化b 其余slave进程通过被广播的形式获得向量b*/
			         MPI_Bcast(&b[0], cols, MPI_FLOAT, master, MPI_COMM_WORLD);
		         /*向各个slave进程发送矩阵a的各行*/
			         int numsent = 0;
		         for (i = 1; i < min(numprocs, rows + 1); i++)
			         {
			             /* 每个slave进程计算一行×一列的结果 这里用MPI_TAG参数标示对应结果向量c的下标+1
			              * MPI_TAG在这里的开始取值范围是1 要把MPI_TAG空出来 作为结束slave进程的标志*/
				             MPI_Send(&a[i - 1][0], cols, MPI_FLOAT, i, ++numsent, MPI_COMM_WORLD);
			         }
		         /*master进程接受其他各进程的计算结果*/
			         for (i = 0; i < rows; i++)
			         {
			             /*类似poll的方法 只要有某个slave进程算出来结果了 MPI_Recv就能返回执行一次*/
				             MPI_Recv(&row_result, 1, MPI_FLOAT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			             /*这里MPI_TAG的范围是1到rows 注意存储结果时下标减1*/
				             c[status.MPI_TAG - 1] = row_result;
			             /*发送矩阵a中没发送完的行 就用刚返回计算结果空出来的那个slave进程 通过status.MPI_SOURCE找到这个空出来的进程*/
				             if (numsent < rows) {
				                 MPI_Send(&a[numsent][0], cols, MPI_FLOAT, status.MPI_SOURCE, numsent + 1, MPI_COMM_WORLD);
				                 numsent = numsent + 1;
				
			}
			             else { /*发送空消息 关闭slave进程*/
				                 float close = 1.0;
				                 MPI_Send(&close, 1, MPI_FLOAT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
				
			}
			         }
		         /*打印乘法结果*/
			         for (j = 0; j < cols; j++)
			             printf("%1.3f\t", c[j]);
		         printf("\n");
		
	}
	     /*slave进程*/
		     else {
		         MPI_Bcast(&b[0], cols, MPI_FLOAT, master, MPI_COMM_WORLD);
		         while (1)
			         {
			             row_result = 0;
			             MPI_Recv(&c[0], cols, MPI_FLOAT, master, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			             if (0 != status.MPI_TAG) {
				                 for (j = 0; j < cols; j++)
					                 {
					                     row_result = row_result + b[j] * c[j];
					                 }
				                 //printf("myid:%d, MPI_TAG:%d, c[0]:%f, row_result:%1.3f\n", myid, status.MPI_TAG,c[0], row_result);
					                 MPI_Send(&row_result, 1, MPI_FLOAT, master, status.MPI_TAG, MPI_COMM_WORLD);
				
			}
			             else {
				                break;
				
			}
			         }
		
	}
	     MPI_Finalize();
	 }
