lui      $1, 0x00000000 
ori      $t4,$1,0x00000014     # t4(Input register) is initialized to 10.
and      $t0, $0, $0       	   # i = 0
lui      $1, 0x00001001 	   # Address of a
ori      $t1,$1,0x00000010 

loop1:
    sll      $t2, $t0, 2       # byte offset for ith element
    add      $t2, $t2, $t1     # address of a[i]
    sw       $t0, 0($t2)       # put i into a[i]
    addi     $t0, $t0, 1       # increment i
    slt      $t5, $t0, $t4     # is $t0 < $t4 ?
    bne      $t5, $0, loop1    # branch if so

# Sum the array values
    and      $s0, $0, $0       # sum = 0, Result Register
    and      $t0, $0, $0       # i = 0
    add      $t2, $t1, $0      # address of a[i]

loop2:
    lw       $t3, 0($t2)       # load a[i]
    mult	 $t3, $t3		   # multiply a[i] with a[i]
    mflo	 $t3 			   # move product to $t3
    add      $s0, $s0, $t3     # increment sum
    addi     $t0, $t0, 1       # increment i
    addi     $t2, $t2, 4       # increment address of a[i]
    slt      $t5, $t0, $t4     # is $t0 < $t4 ?
    bne      $t5, $0, loop2    # branch if so

# Output Sum
    li       $v0, 1            # Load 1=print_int into $v0
    add      $a0, $s0, $zero   # Load first number into $a0
    jr	$ra