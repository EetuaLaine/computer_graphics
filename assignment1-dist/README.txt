# CS-C3100 Computer Graphics, Fall 2021
# Lehtinen / Aho, Kaskela, Kynkäänniemi
#
# Assignment 1: Introduction

Student name: Eetu Laine
Student number: 587549
Hours spent on requirements (approx.): ~8-10
Hours spent on extra credit (approx.): ~12-14

# First, some questions about where you come from and how you got started.
# Your answers in this section will be used to improve the course.
# They will not be judged or affect your points, but please answer all of them.
# Keep it short; answering shouldn't take you more than 5-10 minutes.

- What are you studying here at Aalto? (Department, major, minor...?) 
Computer Science major, master's degree. 

- Which year of your studies is this? 
6th and final if things go according to plan. 

- Is this a mandatory course for you? 
Yes. 

- Have you had something to do with graphics before? Other studies, personal interests, work? 
No.

- Do you remember basic linear algebra? Matrix and vector multiplication, cross product, that sort of thing? 
Yes. 

- How is your overall programming experience? What language are you most comfortable with? 
Most comfortable with Python, but also got some experience with C++, Scala. Overall 5 years of experience. 

- Do you have some experience with these things? (If not, do you have experience with something similar such as C or Direct3D?)
C++: Yes.
C++11: Yes I think. 
OpenGL: No. 

- Have you used a version control system such as Git, Mercurial or Subversion? Which ones?

I have used Git. 

- Did you go to the technology lecture?

No lectures this year. 

- Did you go to exercise sessions?

No. 

- Did you work on the assignment using Aalto computers, your own computers, or both?

I used my own machine and Visual Studio 2019 Community. 
My computer has an RTX 2060 GPU so performance-wise the code might actually not be as good as it should. 

# Which parts of the assignment did you complete? Mark them 'done'.
# You can also mark non-completed parts as 'attempted' if you spent a fair amount of
# effort on them. If you do, explain the work you did in the problems/bugs section
# and leave your 'attempt' code in place (commented out if necessary) so we can see it.

(Try to get everything done! Based on previous data, virtually everyone who put in the work and did well in the first two assignments ended up finishing the course, and also reported a high level of satisfaction at the end of the course.)

                            opened this file (0p): done
                         R1 Moving an object (1p): done
R2 Generating a simple cone mesh and normals (3p): done
  R3 Converting mesh data for OpenGL viewing (3p): done
           R4 Loading a large mesh from file (3p): done

# Did you do any extra credit work?

(Describe what you did and, if there was a substantial amount of work involved, how you did it. Also describe how to use/activate your extra features, if they are interactive.)

I did version control, made a git repo for the course. I implemented scaling along the x axis with x and z keys, 
object rotation along y axis using y and u keys, fixed the shading (I think), Implemented mouse controls for the camera. The camera
rotates along the y axis when dragging the mouse horizontally, and zooms in and out when dragging the mouse vertically. 
I implemented the automatic rotation when pressing r, and also the viewport correction. I also implemented my own version of the perspective
scaling, and commented out the easy viewport correction. My interpretation of the assignment was to scale down the larger side of the window
based on the ratio of the smaller and larger side. That way stretching the window along one side does not distort the object. I also tried
to implement loading PLY files, but it did not seem to work correctly so I commented it out. 

# Are there any known problems/bugs remaining in your code?

(Please provide a list of the problems. If possible, describe what you think the cause is, how you have attempted to diagnose or fix the problem, and how you would attempt to diagnose or fix it if you had more time or motivation. This is important: we are more likely to assign partial credit if you help us understand what's going on.)

The PLY loading does not work correctly, it produces weird looking spherical objects and the faces do not seem to align. I think it's just due to faulty handling of binary data. 

# Did you collaborate with anyone in the class?

(Did you help others? Did others help you? Let us know who you talked to, and what sort of help you gave or received.)

I did not really interact with others during this round. 

# Any other comments you'd like to share about the assignment or the course so far?

(Was the assignment too long? Too hard? Fun or boring? Did you learn something, or was it a total waste of time? Can we do something differently to help you learn? Please be brutally honest; we won't take it personally.)

I found the exercise to be pretty fun and interesting to do. 
