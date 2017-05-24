# filesys

- In this project, I implemented a file system with basic functionalities. 

- For starters, I created a core loop to get input from user, parse it, call corresponding function and print the output. All of that code can be found in main method.

- In the first part, I copied and altered the implementation of the ls function. I added some error handling. The keypoints were that, while traversing, I needed to record the found places of filename1 and filename2. Also, at the end, for correctness purposes, I needed to check only the used blocks of a file to calculate the difference. This part works fine and without errors.

- In the second part, I altered the code of ls function again. After error handlings, I recorded the place of the filename1, read the corresponding block to memory, edit the data and write the result back to where it was. This part also works great and without errors.

- In this part, I based my code on md function. I added a few error handling clauses. While traversing the directory for empty spaces, I also checked the full spaces to find the input file and recorded its place. The rest of the code works as expected. I changed the part where a directory is created to file creation. That worked fine as it produced new files as expected. Yet, for copying part, for some reason that I couldn't find, my code does not copy the neccessary parts to the newly created file, so in the end I get mixed results. For instance, a copied file called "new" is produced correctly when I checked with compare method, yet other test cases failed. So, this part has some bugs but implemented almost fully working.

