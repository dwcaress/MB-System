C **********************************************************************SPLD   1
C ** PROGRAM SPLOAD - LOADS NAD 1927 AND NAD 1983 DIRECT ACCESS FILES **SPLD   2
C ** U. S. GEOLOGICAL SURVEY - CHIPPEAUX AND LINCK           09/19/89 **SPLD   3
C **********************************************************************SPLD   4
      PROGRAM SPLOAD                                                    SPLD   5
C                                                                       SPLD   6
C     PROGRAM TO COPY NAD 1927 AND NAD 1983 SEQUENTIAL FILES TO DIRECT  SPLD   7
C     ACCESS FILES FOR USE BY GCTP                                      SPLD   8
C                                                                       SPLD   9
      REAL*8 PARM(9)                                                    SPLD  10
      INTEGER*4 I, ID, IZONE, LENGTH                                    SPLD  11
      CHARACTER*32 ZNAME, GETF27, GETF83, PUTF27, PUTF83                SPLD  12
      CHARACTER*3 STAT27, STAT83                                        SPLD  13
C                                                                       SPLD  14
C     IPR    = LOGICAL UNIT NUMBER OF PRINTER                           SPLD  15
C     GETF27 = NAME OF NAD 1927 INPUT FILE                              SPLD  16
C     PUTF27 = NAME OF NAD 1927 OUTPUT FILE                             SPLD  17
C     GETF83 = NAME OF NAD 1983 INPUT FILE                              SPLD  18
C     PUTF83 = NAME OF NAD 1983 OUTPUT FILE                             SPLD  19
C     STAT27 = STATUS OF NAD 1927 OUTPUT FILE                           SPLD  20
C     STAT83 = STATUS OF NAD 1983 OUTPUT FILE                           SPLD  21
C     LENGTH = LENGTH IN BYTES OR WORDS OF OUTPUT RECORDS               SPLD  22
C                                                                       SPLD  23
      PARAMETER (IPR = 6)                                               SPLD  24
      PARAMETER (GETF27 = 'nad1927', GETF83 = 'nad1983')                SPLD  25
      PARAMETER (PUTF27 = 'nad27sp', PUTF83 = 'nad83sp')                SPLD  26
      PARAMETER (STAT27 = 'NEW', STAT83 = 'NEW')                        SPLD  27
      PARAMETER (LENGTH = 108)                                          SPLD  28
C                                                                       SPLD  29
      OPEN(UNIT=2,FILE=GETF27,STATUS='OLD',                             SPLD  30
     .     ACCESS='SEQUENTIAL')                                         SPLD  31
      OPEN(UNIT=3,FILE=PUTF27,STATUS=STAT27,RECL=LENGTH,                SPLD  32
     .     ACCESS='DIRECT')                                             SPLD  33
C                                                                       SPLD  34
      OPEN(UNIT=12,FILE=GETF83,STATUS='OLD',                            SPLD  35
     .     ACCESS='SEQUENTIAL')                                         SPLD  36
      OPEN(UNIT=13,FILE=PUTF83,STATUS=STAT83,RECL=LENGTH,               SPLD  37
     .     ACCESS='DIRECT')                                             SPLD  38
C                                                                       SPLD  39
    1 FORMAT(A32,7X,I1,31X,I4)                                          SPLD  40
    2 FORMAT(3D25.16)                                                   SPLD  41
    3 FORMAT(1X,I3,5X,A32,5X,I4,5X,I1)                                  SPLD  42
C                                                                       SPLD  43
      I = 0                                                             SPLD  44
   10 READ(2,1,END=20) ZNAME, ID, IZONE                                 SPLD  45
      I = I + 1                                                         SPLD  46
      READ(2,2) PARM(1), PARM(2), PARM(3)                               SPLD  47
      READ(2,2) PARM(4), PARM(5), PARM(6)                               SPLD  48
      READ(2,2) PARM(7), PARM(8), PARM(9)                               SPLD  49
      WRITE(3,REC=I) ZNAME, ID, PARM                                    SPLD  50
      WRITE(IPR,3) I, ZNAME, IZONE, ID                                  SPLD  51
      GO TO 10                                                          SPLD  52
C                                                                       SPLD  53
   20 I = 0                                                             SPLD  54
   30 READ(12,1,END=40) ZNAME, ID, IZONE                                SPLD  55
      I = I + 1                                                         SPLD  56
      READ(12,2) PARM(1), PARM(2), PARM(3)                              SPLD  57
      READ(12,2) PARM(4), PARM(5), PARM(6)                              SPLD  58
      READ(12,2) PARM(7), PARM(8), PARM(9)                              SPLD  59
      WRITE(13,REC=I)  ZNAME, ID, PARM                                  SPLD  60
      WRITE(IPR,3) I, ZNAME, IZONE, ID                                  SPLD  61
      GO TO 30                                                          SPLD  62
C                                                                       SPLD  63
   40 CLOSE(2)                                                          SPLD  64
      CLOSE(3)                                                          SPLD  65
      CLOSE(12)                                                         SPLD  66
      CLOSE(13)                                                         SPLD  67
C                                                                       SPLD  68
      STOP                                                              SPLD  69
      END                                                               SPLD  70
