      subroutine label(x, y, height, theta, text, ntext)
c$$$$ calls letter
c     calls letter using text created in a C routine
      character*7 text
      real x, y, height, theta
       call letter(x,y,height,theta,text(1:ntext))
      return
      end
c
      subroutine justify(s,height,text,ntext)
c$$$$ calls justy
      character*7 text
      real s(4),height
      integer ntext
      call justy(s,height,text(1:ntext))
      return
      end
c
