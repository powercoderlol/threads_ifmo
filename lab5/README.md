# Execution
1. Execute on host machine `docker-compose up -d`
2. Execute on host machine `docker exec -it %ID% /bin/bash` where ID - container identifier
3. `/scripts` - directory contains run scripts for over yarn and standalone modes

# Results
1. Execute `hadoop fs -cat /%folder_name%/part-00000` where folder_name is output1/output2/output3 to get your own results
2. RESULTS.DAT contains expected output