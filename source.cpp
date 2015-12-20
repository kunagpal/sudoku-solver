#include<iostream.h>
#include<conio.h>
#include<time.h>
#include<process.h>
#include<stdlib.h>

int un[81], cp, s1[81], s2[81], csc[81], cc[512], cb[81], bcn[9][9], lc[81], ls, lce[81], lr[81], lco[81], lb[81];

void r(int currentcell) 
{
    int i, j, k, l, row, column, box;
    /*  Each unsolved cell that shares the same row, column or box with the current cell will lose the value last placed in the current cell as candidate
        The current cell is saved, along with the positions of the cells losing the candidate, related to their row, column or box.
        If the brute force handler needs to backtrack, the undo() routine uses this information to restore those candidates */
    lce[++ls] = currentcell;        //save current cell, this is the last altered cell
    lr[ls] = lco[ls] = lb[ls] = 0;  //clear box log
    row = currentcell / 9;          //calculate the row where the current cell belongs to
    column = currentcell % 9;       //calculate the column where the current cell belongs to
    box = cb[currentcell];          //get the boxnumber for the current cell
    j = 1;                          //prepare candidate bitmask
    
    if (s1[currentcell] > 1) 
    {
        for (i = 1; i < s1[currentcell]; ++i) 
        {
            j <<= 1;    //update candidate bitmask
        }
    }
    
    for (i = 0; i < cp; ++i)//scan all unsolved cells
    {
        if ((csc[un[i]] & j) > 0)//test if candidate is present
        {
            if (un[i] / 9 == row)//check if it shares row with current cell
            {
                l = 1;//prepare position bitmask
                for (k = 0; k < 9; ++k)//search position in row
                {
                    if ((row * 9) + k == un[i])//if position found...
                    {
                        lr[ls] |= l;//set position bit in row log
                        csc[un[i]] &= (511 ^ j);//remove candidate from list
                        break;
                    }

                    l <<= 1;//update position bitmask
                }
            }
            else if (un[i] % 9 == column)//check if it shares column with current cell
            {
                l = 1;//prepare position bitmask

                for (k = 0; k < 9; ++k)//search position in column
                {
                    if ((k * 9) + column == un[i])//if position found...
                    {
                        lco[ls] |= l;//set position bit in column log
                        csc[un[i]] &= (511 ^ j);//remove candidate from list
                        break;
                    }
                    
                    l <<= 1;//update position bitmask
                }
            }
            else if (cb[un[i]] == box)//check if it shares box with current cell
            {
                l = 1;//prepare position bitmask

                for (k = 0; k < 9; ++k)         //search position in box
                {
                    if (bcn[box][k] == un[i])   //if position found...
                    {
                        lb[ls] |= l;            //set position bit in box log
                        csc[un[i]] &= (511 ^ j);//remove candidate from list
                        break;                  //stop position search
                    }
                    
                    l <<= 1;//update position bitmask
                }
            }
        }
    }
}

void u() 
{
    int i, j, k, l, row, column, box;
    //This routine uses the information stored by r[] to replace the last removed candidates from the unsolved cells whenever the brute force
    //handler needs to backtrack
    k = s1[lce[ls]];//get the candidate value
    s1[lce[ls]] = 0;//clear the s1
    row = lce[ls] / 9;//calculate row
    column = lce[ls] % 9;//calculate column
    box = cb[lce[ls]];//get boxnumber
    j = 1;//prepare candidate bitmask

    if (k > 1) 
    {
        for (i = 1; i < k; ++i) 
        {
            j <<= 1;    //update candidate bitmask
        }
    }
    
    l = 1;//prepare position bitmask
    
    for (i = 0; i < 9; ++i)//scan all positions
    {
        if ((lr[ls] & l) > 0)           //if position is in row log then replace candidate
        {
            csc[(row * 9) + i] |= j;
        }
        if ((lco[ls] & l) > 0)         //if position is in column log then replace candidate
        {
            csc[(i * 9) + column] |= j;
        }
        if ((lb[ls] & l) > 0)                   //if position is in box log then replace candidate
        {
            csc[bcn[box][i]] |= j;
        }

        l <<= 1;  //update position bitmask
    }
    
    --ls;   //decrease logcounter (for next backtrack, if needed)
}

int ns() 
{
    int i, j, k, row, column, box, foundnaked = 0;//clear return value

    for (i = 0; i < 81; ++i)//scan all cells
    {
        if (cc[csc[i]] == 1)                //search for unsolved cell with only one possible candidate (naked single)
        {
            foundnaked = 1;     //set return value
            row = i / 9;        //calculate row
            column = i % 9;     //calculate column
            box = cb[i];        //get boxnumber
            j = 1;              //prepare bitmask
            
            for (k = 0; k < 9; ++k)//scan candidate list
            {
                if ((csc[i] & j) > 0)//test bit for available candidate
                {
                    s1[i] = k + 1;//assign to s1
                    break;//stop scanning candidate list
                }
                
                j <<= 1;//update bitmask
            }
            
            csc[i] = 0;//clear candidate list from naked single cell
            
            for (k = 0; k < 9; ++k)//for each position in same row, column or box
            {
                csc[(row * 9) + k] &= (511 ^ j);                    //update csc for each cell in same row
                csc[(k * 9) + column] &= (511 ^ j);  //update csc for each cell in same column
                csc[bcn[box][k]] &= (511 ^ j);  //update csc for each cell in same box
            }
            
            break;
        }
    }
    
    return (foundnaked);
}

int cu() 
{
    int i, j, k, solvedcells, row, column, box, currentcell, counts1s, boundary;
    /* This routine will brute force solve the puzzle contained in s2[] since s2[] is based upon a solved grid, at least one s1 will be found
        When found the first and hopefully the only s1, the last solved cell will be rejected so the search will continue. If another s1 is found
        the routine will be halted and counts1s will be 2, indicating that there are at least 2 s1s, and the brute force loop will be stopped.
        If there is only one s1, this brute force will eventually backtrack beyond the first solved cell, can't have that!
        Therefore, a boundary is set to prevent this, and ends the brute force loop. At last, the value from counts1s is returned (1=1 s1, 2=at least 2 s1s)
        initiation */

    for (i = 0; i < 81; ++i)
    {
        s1[i] = s2[i];  //transfer the s2 puzzle to s1[]
        csc[i] = 511;   //set all candidates available for all cells
        lc[i] = 0;  //clear last candidate indicator for all cells
    }
    
    solvedcells = 0;//setup solved cells counter
    //update csc[]
    for (i = 0; i < 81; ++i)//scan s2[]
    {
        if (s1[i] > 0)//for each solved cell...
        {
            ++solvedcells;//update solved cells counter
            //update cells candidates
            //remove candidates for each solved cell
            csc[i] = 0;
            row = i / 9;//calculate row from solved cell
            column = i % 9;//calculate column from solved cell
            box = cb[i];//get boxnumber from solved cell
            j = 1;//prepare candidate bitmask

            if (s1[i] > 1) 
            {
                for (k = 1; k < s1[i]; ++k) 
                {
                    j <<= 1;    //update candidate bitmask
                }
            }
            
            for (k = 0; k < 9; ++k)//for all positions in row, column or box...
            {
                csc[(row * 9) + k] &= (511 ^ j); //remove candidate from cells in row
                csc[(k * 9) + column] &= (511 ^ j); //remove candidate from cells in column
                csc[bcn[box][k]] &= (511 ^ j); //remove candidate from cells in box
            }
        }
    }
    
    //solve naked singles first
    while (ns() == 1) 
    {
        ++solvedcells;  //update solved cells counter
    }
    if (solvedcells == 81)//check if puzzle is solved
    {
        return (counts1s = 1);//return
    }
    //collect unsolved cells
    cp = -1;//reset unsolved cell pointer
    
    for (i = 0; i < 81; ++i)//scan s2[]
    {
        if (!s1[i])//for each unsolved cell...
        {
            //collect all unsolved cells
            //increase cp for each unsolved cell
            //transfer cellnumber for each unsolved cell to array un[]
            un[++cp] = i;
        }
    }
    
    boundary = cp + 1;//set boundary
    ls = -1;//reset log stepcounter
    counts1s = 0;//clear s1 counter
    
    while (cp < boundary)//do brute force loop but don't let it backtrack beyond the first unsolved cell
    {
        if (cp == -1)//check cp, if -1 then a s1 is found
        {
            ++counts1s; //s1 found, increase s1 counter
            //if this is the second s1 then break the brute force loop
            if (counts1s == 2) 
            {
                break;
            }
            //at this point only one s1 is found, now reject
            //the last solved cell by increasing cp, so the search
            //can continue for other s1s, if any
            ++cp;
        }
        
        //FROM HERE THE CODE IS THE SAME AS IN THE BRUTE FORCE LOOP FROM g()
        
        currentcell = un[cp];
        if (!s1[un[cp]])
        {
            for (i = 0; i < cp; ++i)
            {
                if (cc[csc[un[i]]] < cc[csc[un[cp]]])
                {
                    un[i] += un[cp];
                    un[cp] = un[i] - un[cp];
                    un[i] -= u[cp];
                }
            }

            currentcell = un[cp];
            j = 1;

            for (i = 0; i < 9; ++i) 
            {
                if ((csc[currentcell] & j) > 0) 
                {
                    s1[currentcell] = i + 1;
                    break;
                }
                
                j <<= 1;
            }
            
            if (cc[csc[currentcell]] == 1) 
            {
                lc[currentcell] = 1;
            }
            
            r(currentcell);
            j = cp--;
            
            if (cp > -1) 
            {
                for (i = 0; i < j; ++i)
                {
                    if (!csc[un[i]])
                    {
                        ++cp;
                        break;
                    }
                }
            }
        }
        else 
        {
            if (lc[currentcell] == 1)
            {
                u();
                lc[currentcell] = 0;
                ++cp;
            }
            else 
            {
                j = 1;
                k = s1[currentcell];
                u();
                
                for (i = 0; i < k; ++i)
                {
                    j <<= 1;
                }

                for (i = k; i < 9; ++i)
                {
                    if ((csc[currentcell] & j) > 0)
                    {
                        s1[currentcell] = i + 1;
                        break;
                    }
                    
                    j <<= 1;
                }
                
                lc[currentcell] = 1;
                if (s1[currentcell] < 9) 
                {
                    for (i = s1[currentcell]; i < 9; ++i) 
                    {
                        j <<= 1;

                        if ((csc[currentcell] & j) > 0)
                        {
                            lc[currentcell] = 0;
                            break;
                        }
                    }
                }

                r(currentcell);
                j = cp--;

                if (cp > -1)
                {
                    for (i = 0; i < j; ++i)
                    {
                        if (!csc[un[i]])
                        {
                            ++cp;
                            break;
                        }
                    }
                }
            }
        }
    }
    //brute force loop ended, return counts1s
    return (counts1s);
}

void in() 
{
    int i, j, k, l, cell, box;
    //possible candidates for each cell are stored bitwise in array csc[].If candidate 1 is present then bit 0 is set,
    //if candidate 2 is present then bit 1 is set, and so on... The array cc[] will return the number of candidates
    //if feeded with any cell's csc[] Here, the array cc[] is initiated.

    for (i = 0; i < 512; ++i)
    {
        j = i;
        cc[i] = 0;

        while (j > 0)
        {
            if (j % 2) 
            {
                ++cc[i];
            }
            
            j = (j - (j % 2)) >> 1;
        }
    }

    //Boxes are adressed 0 to 8 from top left to bottom right. The array cb[] will return the boxnumber for
    //any cell, it acts as some kind of boxmap. The array bcn[][] holds the 9 cellnumbers
    //for each box. Here we initiate both arrays

    for (i = 0; i < 3; ++i)
    {
        for (j = 0; j < 3; ++j) 
        {
            for (k = 0; k < 3; ++k)
            {
                for (l = 0; l < 3; ++l)
                {
                    cell = (j * 27) + (i * 3) + (l * 9) + k;
                    box = (j * 3) + i;
                    cb[cell] = box;
                    bcn[box][(l * 3) + k] = cell;
                }
            }
        }
    }
}

void g(void) 
{
    int i, j, k, l, currentcell;

    for (i = 0; i < 81; ++i)    //prepare base data for empty grid
    {
        un[i] = i;//collect all cellnumbers as unsolved cells
        csc[i] = 511;//set all candidates as available for all cells
        s1[i] = lc[i] = 0;//clear last candidate indicator
    }
    //mix up the order of the cells kept in un[]
    
    for (i = 0; i < 500; ++i)//500 times will be enough
    {
        j = (rand() % 81);//generate a random number between 0 and 80
        k = (rand() % 81);//generate another between 0 and 80
        //swap cell adresses kept in un[] pointed by i and j
        un[i] += un[j];
        un[j] = un[i] - un[j];
        un[i] -= un[j];
    }

    cp = 80;//initiate unsolved cells pointer
    ls = -1;//initiate candidate update counter
    //start brute force loop until grid is filled
    while (cp > -1)//if cp=-1 then the grid is filled
    {
        currentcell = un[cp];//get current cell from array un
        //check if the unsolved cell pointed by cp is already under examination
        if (!s1[currentcell])//if 0 then the cell is not already under examination
        {
            //seek through all unsolved cells for the one with the least available candidates
            //kinda bubble sort
            for (i = 0; i < cp; ++i) 
            {
                if (cc[csc[un[i]]] < cc[csc[un[cp]]]) 
                {
                    un[i] += un[cp];
                    un[cp] = un[i] - un[cp];
                    un[i] -= un[cp];
                }
            }
            
            currentcell = un[cp]; //copy cellnumber with least available candidates to currentcell
            //seek for the first available candidate
            j = 1;//prepare bitmask

            for (i = 0; i < 9; ++i)//scan candidates
            {
                if ((csc[currentcell] & j) > 0)//test with bitmask
                {
                    s1[currentcell] = i + 1;//set as tempararely s1
                    break;
                }
                
                j <<= 1;//update bitmask
            }
            
            //check if there is only one candidate, and if so set lc indicator
            if (cc[csc[currentcell]] == 1) 
            {
                lc[currentcell] = 1;
            }
            
            r(currentcell);//update candidates in unsolved cells sharing row, column or box of currentcell
            j = --cp;//copy current value of cp to j

            if (cp > -1)                //if there are still cells unsolved check if there are empty cells among them
            {
                for (i = 0; i < j; ++i)//scan all unsolved cells
                {
                    //check for empty cell
                    if (!csc[un[i]])//if 0 then empty cell
                    {
                        //empty cell caused by last altered cell
                        ++cp;//increase cp, so it will point to the last altered cell again
                        break;
                    }
                }
            }
        }
        else//currentcell is allready under examination, apparantly not the right s1
        {
            //check if the current value was the last available candidate
            if (lc[currentcell] == 1)//check last candidate indicator
            {
                //no more candidates available for the current cell
                u();//undo the last candidate update
                lc[currentcell] = 0;//clear the last candidate indicator
                ++cp;//increase cp (prepare for previous cell)
            }
            else//try next available candidate for the current cell
            {
                j = 1;//prepare bitmask
                k = s1[currentcell];//get last used candidate
                u();//undo the last candidate update
                
                for (i = 0; i < k; ++i) 
                {
                    j <<= 1;    //update bitmask
                }
                
                //seek for next available candidate
                for (i = k; i < 9; ++i)//scan next candidates
                {
                    if ((csc[currentcell] & j) > 0)//test with bitmask
                    {
                        //next available candidate found
                        s1[currentcell] = i + 1;//set as temperarely s1
                        break;
                    }
                    
                    j <<= 1;//update bitmask
                }
                
                lc[currentcell] = 1;//by default, set last candidate indicator
                //if current used candidate is not 9 then maybe it's not the last candidate used
                if (s1[currentcell] < 9)//check if there are more available candidates
                {
                    for (i = s1[currentcell]; i < 9; ++i)//scan other candidates
                    {
                        j <<= 1;//update bitmask

                        if ((csc[currentcell] & j) > 0)//test with bitmask
                        {
                            //more candidates available
                            lc[currentcell] = 0;//clear last candidate indicator
                            break;
                        }
                    }
                }
                
                r(currentcell);//update candidates in unsolved cells sharing row, column or box of currentcell
                j = --cp;//copy current value of cp to j
                
                if (cp > -1)    //if there are still cells unsolved check if there are empty cells among them
                {
                    for (i = 0; i < j; ++i)//scan all unsolved cells
                    {
                        //check for empty cell
                        if (!csc[un[i]])//if 0 then empty cell
                        {
                            //empty cell caused by last altered cell
                            ++cp;//increase cp, so it will point to the last altered cell again
                            break;
                        }
                    }
                }
            }
        }
    }
}

void c() 
{
    int i, j, k, l, s1backup[81], countconcealedcells, currentcell, mixedorder[41];
    /*  This routine will remove values from the random ordered cell list, and checks
        for uniqueness. To gain an 180° rotational s2 cells are removed in pairs.
        therefore only the first half of the cells are adressed directly. The 180° rotational
        opposite cell is calculated: opposite cell = 80 - cell */

    for (i = 0; i < 81; ++i)
    {
        s1backup[i] = s1[i];        //make a backup of the s1, since the array s1 is used by the routine that checks the uniqueness
        s2[i] = s1[i];          //copy the s1 to handle the cells to conceal

        if (i < 41)             //prepare an array to mix up the order of the first half of the cells
        {
            mixedorder[i] = i; //setup first half of cells
        }
    }
    
    for (i = 0; i < 250; ++i)//mix up the order of the cellnumbers kept in array mixedorder[]
    {
        j = (rand() % 41);
        k = (rand() % 41);
        mixedorder[j] += mixedorder[k];
        mixedorder[k] = mixedorder[j] - mixedorder[k];
        mixedorder[j] -= mixedorder[k];
    }

    countconcealedcells = 0;//clear concealed cell counter

    for (i = 0; i < 41; ++i)//scan half of the cells
    {
        if (countconcealedcells == 58) 
        {
            break;  //stop concealing if 58 cells are concealed succesful
        }
        
        currentcell = mixedorder[i];            //get next cell
        s2[currentcell] = s2[80 - currentcell] = 0; //conceal that cell (remove the value)
        //check uniqueness
        if (cu() == 1)//if return value is 1, then there was one unique s1
        {
            countconcealedcells += 2;
        }
        else//if return value is not 1, then there was more than one s1
        {
            s2[currentcell] = s1backup[currentcell];                //restore the last concealed cells
            s2[80 - currentcell] = s1backup[80 - currentcell];
        }
    }
    
    //all possible cells to conceal with a maximum of 58 are concealed, and the s2 to solve is in array s2[]
    //now restore the s1

    for (i = 0; i < 81; ++i) 
    {
        s1[i] = s1backup[i];
    }
}

void s() 
{
    int i, j, k, l;
    cout << "\n Sudoku Solution\n----------- -----------\n";
    for (i = 0; i < 3; ++i) 
    {
        for (j = 0; j < 3; ++j) 
        {
            for (k = 0; k < 3; ++k) 
            {
                for (l = 0; l < 3; ++l) 
                {
                    cout << s2[(i * 27) + (j * 9) + (k * 3) + l];
                }
                
                cout << ' ';
            }
            
            cout << " ";
            
            for (k = 0; k < 3; ++k) 
            {
                for (l = 0; l < 3; ++l) 
                {
                    cout << s1[(i * 27) + (j * 9) + (k * 3) + l];
                }
                
                cout << ' ';
            }
            
            cout << '\n';
        }
        
        cout << '\n';
    }
    
    cout << "\n\n";
}

void main() 
{
    clrscr();
    start:
    char ch;
    srand((unsigned) time(NULL));
    in(); //initiate arrays and base data
    g(); //generate random solution
    c(); //conceal s1 to regular s2
    s(); //set result on screen
    cout << "\nAgain?";
    cin >> ch;
    
    if (ch == 'y')
    {
        goto start;
    }   
    else exit(0);
    getch();
}