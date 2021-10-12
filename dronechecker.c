// 6521724 zy21724 Jingyu LUO
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <assert.h>

	//data type of waypoint in a flight plan
	struct two_ints_list {
		int x;
		int y;
		struct two_ints_list *next;
	};
	typedef struct two_ints_list Flightplan;

	//data type of circles represents no-fly zones
	struct three_ints_list{
		int x;
		int y;
		int r;
		struct three_ints_list *next;
	};
	typedef struct three_ints_list Noflyzone;

	//values returned from list_get_X functions
	int FPreturnX, FPreturnY;
	int NFZreturnX, NFZreturnY, NFZreturnR;

int Intersect(int Cx, int Cy, int r, int Ax, int Ay, int Bx, int By);
int segment_in(int Cx, int Cy, int r, int Ax, int Ay, int Bx, int By);
int distance_from_waypointA(int Ax, int Ay, int Cx, int Cy);

int list_append_FP(Flightplan **start, int X, int Y);
int list_append_NFZ(Noflyzone **start, int X, int Y, int R);
int list_get_FP(Flightplan *start, int n);
int list_get_NFZ(Noflyzone *start, int n);
void list_free_FP(Flightplan **start);
void list_free_NFZ(Noflyzone **start);



int main(int argc, char const *argv[])
{	
	//define variables of type Noflyzone and Flightplan
	Noflyzone *area = NULL;
	Flightplan *waypoint = NULL;
	//individual variables in area and waypoint
	int areaX, areaY, radius;
	int waypointAx, waypointAy, waypointBx, waypointBy;

	if (argc != 3)
	{
		printf("Invalid command line arguments. Usage: <Noflyzone> <flightplan>\n");
		exit(0);
	}

	//get file names from command line and assign file handles 
	FILE *noflyPtr;
	FILE *planPtr;
	noflyPtr = fopen(argv[1], "r");
	planPtr = fopen(argv[2], "r");
	if (noflyPtr == NULL)
	{
		perror("Cannot open nofly file ");
		exit(1);
	}
	else if (planPtr == NULL)
	{
		perror("Cannot open flightplan file ");
		exit(1);
	}

	//read data from no-fly area and store in a structure	
	char string[200];
	char first_char;
	while(!feof(noflyPtr))
	{
		fscanf(noflyPtr, "%c", &first_char);
		//char is neither # nor a decimal number nor a newline character
		if (first_char != '#' && !(first_char >= 48 && first_char <= 57) && first_char != '\n')  
		{
			printf("no-fly area file invalid.\n");
			fclose(noflyPtr);
			fclose(planPtr); 
			exit(3);
		}
		//it is an integer
		else if (first_char >= 48 && first_char <= 57)
		{	
			fseek(noflyPtr, -1, SEEK_CUR);
			fscanf(noflyPtr, "%d%d%d", &areaX, &areaY, &radius);
			//check if successfully read in
			if ((areaX < 0 || areaX >= 10000) || (areaY < 0 || areaY >= 10000) || (radius < 0 || radius >= 10000))
			{
				printf("Coordiates or radius of no-fly area out of bound\n");
				fclose(noflyPtr);
				fclose(planPtr);
				exit(6);
			}
			list_append_NFZ(&area, areaX, areaY, radius);
			if (list_append_NFZ(&area, areaX, areaY, radius) == 0)
			{
				printf("Unable to allocate memory.\n");
				fclose(noflyPtr);
				fclose(planPtr);
				exit(5);
			}
		}
		//if a line starts with #
		else if (first_char == '#') 
		{
			fscanf(noflyPtr,"%[^\n]%*c", string); //ignore this line 
		}		
	}


	while(!feof(planPtr))	
	{								
		fscanf(planPtr, "%c", &first_char);
		//char is neither # nor a decimal number nor a newline character
		if (first_char != '#' && !(first_char >= 48 && first_char <= 57) && first_char != '\n')  
		{
			printf("Flight plan file invalid.\n"); 
			list_free_NFZ(&area);
			fclose(noflyPtr);
			fclose(planPtr);
			exit(3);
		}
		//it's an integer
		else if (first_char >= 48 && first_char <= 57)
		{
			fseek(planPtr, -1, SEEK_CUR);
			fscanf(planPtr, "%d%d", &waypointAx, &waypointAy);
			
			if ((waypointAx < 0 || waypointAx >= 10000) || (waypointAy < 0 || waypointAy >= 10000))
			{
				printf("Coordiante of flight plan out of bound\n");
				list_free_NFZ(&area);
				fclose(noflyPtr);
				fclose(planPtr);
				exit(6);
			}
			list_append_FP(&waypoint, waypointAx, waypointAy);	
		}
		//if a line starts with #
		else if (first_char == '#') 
		{
			//ignore this line 
			fscanf(planPtr,"%[^\n]%*c", string); 
		}
	}

int count1 = 0;
//check if there are 2 consecutive way-points have the same coordinates
int a1, a2, b1, b2;	
while(list_get_FP(waypoint, count1) != 0)
{	
	count1++;
}	
for (int i = 0; i < count1 - 1;)
{
	list_get_FP(waypoint, i);
	a1 = FPreturnX;
	a2 = FPreturnY;
	i++;

	list_get_FP(waypoint, i);
	b1 = FPreturnX;
	b2 = FPreturnY;		
	if (a1 == b1 && a2 == b2)
	{
		printf("Two consecutive way-points have the same coordinates, please revise your flight plan.\n");
		list_free_FP(&waypoint);
		list_free_NFZ(&area);
		fclose(noflyPtr);
		fclose(planPtr);
		exit(6);
	}
}

//start to check if the drone enters any no-fly area
	int count2 = 0;
	int count_invalid = 0;
	int d;
	int min = 99999;
	int the_closest_circle[2];
	while(list_get_NFZ(area, count2) != 0)
	{
		count2++;
	}
	//iteratively check each segment
	for (int i = 0; i < count1 - 1;)
	{
		list_get_FP(waypoint, i);
		waypointAx = FPreturnX;
		waypointAy = FPreturnY;
		i++;
		
		list_get_FP(waypoint, i);
		waypointBx = FPreturnX;
		waypointBy = FPreturnY;

		//for each segment, iteratively check each no-fly area
		for (int j = 0; j < count2 - 1; j++)
		{
			list_get_NFZ(area, j);
			areaX = NFZreturnX;
			areaY = NFZreturnY;
			radius = NFZreturnR;
			//check if the drone enters invalid area
			if (segment_in(areaX, areaY, radius, waypointAx, waypointAy, waypointBx, waypointBy) || Intersect(areaX, areaY, radius, waypointAx, waypointAy, waypointBx, waypointBy))
			{
				d = distance_from_waypointA(waypointAx, waypointAy, areaX, areaY);
				if (d < min)
				{
					min = d;
					the_closest_circle[0] = areaX;
					the_closest_circle[1] = areaY;
				}
				//invalid area exists
				count_invalid++;
			}
		}
		//for this segment, once there is a invalid area, count_invalid must be greater than 0
		if (count_invalid > 0)
		{
			printf("Invalid flight plan\n");
			printf("Enters restricted area around (%d, %d)\n", the_closest_circle[0], the_closest_circle[1]);
			list_free_FP(&waypoint);
			list_free_NFZ(&area);
			fclose(noflyPtr);
			fclose(planPtr);
			exit(4);
		}
	}
	list_free_FP(&waypoint);
	list_free_NFZ(&area);
	fclose(noflyPtr);
	fclose(planPtr);
	//no invalid segment for every no-fly zone
	printf("Flight plan valid.\n");
	exit(0);	
}


int distance_from_waypointA(int Ax, int Ay, int Cx, int Cy)
{
	double distance;
	distance = sqrt((Ax - Cx) * (Ax - Cx) + (Ay - Cy) *(Ay - Cy));
	return distance;
}

int Intersect(int Cx, int Cy, int r, int Ax, int Ay, int Bx, int By)
{
	long X;
	long Y;
	long Z;
	long Discrim;
	X = (Ax * Ax) - (2 * Ax * Bx) + (Bx * Bx) + (Ay * Ay) - (2 * Ay * By) + (By * By);
	Y = (2 * Cx * Bx) - (2 * Cx * Ax) - (2 * Cy * Ay) + (2 * Cy * By) + (2 * Ax * Bx) + (2 * Ay * By) - (2 * Bx * Bx) - (2 * By * By);
	Z = (Cx * Cx) - (2 * Cx * Bx) + (Cy * Cy) - (2 * Cy * By) - (r * r) + (Bx * Bx) + (By * By);
	Discrim = Y * Y - 4 * Z * X;

	if (Discrim < 0)
	{
		return 0;
	}
	else
	{	
		double t1;
		double t2;
		t1 = (-Y + sqrt(Discrim)) / (2 * X);
		t2 = (-Y - sqrt(Discrim)) / (2 * X);

		if ((0 <= t1 && t1 <= 1) || (0 <= t2 && t2 <= 1))
		{
			return 1;		
		}
		else
		{
			return 0;
		}	
	}
}

//check if both the waypoints are in the no-fly area or on the edge of it
int segment_in(int Cx, int Cy, int r, int Ax, int Ay, int Bx, int By)
{
	double distanceA;
	double distanceB;
	distanceA = sqrt((Cx - Ax) * (Cx - Ax) + (Cy - Ay) * (Cy - Ay));
	distanceB = sqrt((Cx - Bx) * (Cx - Bx) + (Cy - By) * (Cy - By));

	if (distanceA <= r && distanceB <= r)
	{	
		return 1;
	}
	else
	{
		return 0;
	}		
}

//taken from int_list.c(complete) on moodle
//change: name IntList to Flightplan, name of function to list_append_FP
//add another integer argument, and change the name value to x, val to X 
int list_append_FP(Flightplan **start, int X, int Y)
{
   if(*start == NULL)
   {
      Flightplan *n = malloc(sizeof(Flightplan));
      if(n == NULL)
      {
      printf("Unable to allocate memory\n");
	 	return 0;
      }
      n->x = X;
      n->y = Y;
      n->next = NULL;
      *start = n;
   }
   else
   {
      Flightplan *cur = *start;
      while(cur->next != NULL)
      {
	     cur = cur->next;
      }

      Flightplan *n = malloc(sizeof(Flightplan));
      if(n == NULL)
      {
       printf("Unable to allocate memory\n");
	     return 0;
      }

      n->x = X; 
      n->y = Y;   
      n->next = NULL;    
      cur->next = n;     
   }
   return 1;
}

//taken from int_list.c(complete) on moodle
//change: name IntList to Noflyzone, function name to list_append_NFZ
//add another two integer arguments, and change the name value to x, val to X 
int list_append_NFZ(Noflyzone **start, int X, int Y, int R)
{
   if(*start == NULL)
   {
      Noflyzone *n = malloc(sizeof(Noflyzone));
      if(n == NULL)
      {
	 	return 0;
      }
      n->x = X;
      n->y = Y;
      n->r = R;
      n->next = NULL;
      *start = n;
   }
   else
   {
      Noflyzone *cur = *start;
      while(cur->next != NULL)
      {
	     cur = cur->next;
      }

      Noflyzone *n = malloc(sizeof(Noflyzone));
      if(n == NULL)
      {
	     return 0;
      }

      n->x = X; 
      n->y = Y;
      n->r = R;   
      n->next = NULL;    
      cur->next = n;     
   }
   return 1;
}

//taken from int_list.c(complete) on moodle
//changed the code in if(n == 0), argument and function names
int list_get_FP(Flightplan *start, int n)
{
   Flightplan *cur = start;
	if(n < 0)
    {
      return 0;
   	}
    while(cur != NULL)
   	{
    	if(n == 0)
      	{
	 		FPreturnX = cur->x;
	 		FPreturnY = cur->y;
	 		return 1;
      	}
      	else
      	{
	 		n--;
	 		cur = cur->next;
      	}
   	}
   	return 0;	
}

//taken from int_list.c(complete) on moodle
//changed the code in if(n == 0), argument and function names
int list_get_NFZ(Noflyzone *start, int n)
{
   Noflyzone *cur = start;
	if(n < 0)
    {
      return 0;
   	}
    while(cur != NULL)
   	{
    	if(n == 0)
      	{
	 		NFZreturnX = cur->x;
	 		NFZreturnY = cur->y;
	 		NFZreturnR = cur->r;
	 		return 1;
      	}
      	else
      	{
	 		n--;
	 		cur = cur->next;
      	}
   	}
   	return 0;	
}

//taken from int_list.c(complete) on moodle
//changed name of function and argument
void list_free_FP(Flightplan **start)
{
   Flightplan *cur = *start;   
   while(cur != NULL)
   {
      Flightplan *temp = cur->next;    
      free(cur);
      cur = temp;                   
   }
   *start = NULL;
}

//taken from int_list.c(complete) on moodle
//changed name of function and argument
void list_free_NFZ(Noflyzone **start)
{
   Noflyzone *cur = *start;   
   while(cur != NULL)
   {
      Noflyzone *temp = cur->next;    
      free(cur);
      cur = temp;                   
   }
   *start = NULL;
}
