
/****************************************************************************************************************************************
 *                                            OS Assignment - 2  ( P2P FILE SHARING APPLICATION )                                        *                                                  
 *                                                                                                                                       *
 *                                                                                                                                       *
 *                                                                                                                                       *
 *                      Name :  Abhisek  Mohapatra                                       Roll Number : 2020201020                        *                                                           
 *                                                                                                                                       *
 *                                                                                                                                       *
 *                                            Last Update :  Nov 8 5:24 pm                                                               *
 *                                                                                                                                       *                                                                                                                                                                                                                                   
 * ****************************************************************************************************************************************/

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <pthread.h>
#include <string>
#include <vector>
#include <iterator>
#include <set>
#include <unordered_map>
#include <utility>
#include <openssl/sha.h>
#include <fcntl.h>

#define chunk_size 524288

using namespace std;

set<string> uid_check;
int login_flag = 0;

unordered_map<string, string> groupAdmin;        // groupId,userId
unordered_map<string, set<string>> groupMembers; // groupId,Members of the group(User Id)
unordered_map<string, set<string>> pend_request; //  groupId,Members of the group
//unordered_map<string,vector<vector<string>>> uploadList;  // groupId -> file path, file size, sha hash, port_no

struct FILES
{
    string user_id;
    string file_path;
    string file_size;
    string sha_hash;
    string port_no;
    string file_name; // New
    string ip_adr;    // New
    string no_chunks; // New
};

vector<struct FILES> fileInfo;
unordered_map<string, vector<struct FILES>> uploadFileGrpId; // groupId -> file path, file size, sha hash, port_no//

/*struct clientDetails
{
    int user_id;
    int user_pwd;
    int loginBit=0;
} 

struct clientDetails cd[10];  */
unordered_map<string, pair<string, int>> clientDetails;
unordered_map<string, pair<string, string>> clientIpPortDetails;

string getFileName(string path)
{
    cout << path << endl;

    unsigned long long int i = path.length() - 1;
    unsigned long long int count = 0;
    while (path[i] != '/')
    {
        i--;
        count++;
    }

    string fileString = path.substr(i + 1, count);
    cout << fileString << endl;

    return fileString;
}

string getTotalChunks(string file_size)
{
    // #define chunk_size 524288

    double chunks = stod(file_size) / stod(to_string(chunk_size));

    return to_string(chunks);
}

string getUserId(string cli_port1)
{
    string k = "";
    pair<string, string> p;

    for (auto it = clientIpPortDetails.begin(); it != clientIpPortDetails.end(); ++it)
    {
        k = it->first;
        p = it->second;
        if (p.second == cli_port1)
            return k;
    }

    return k;
}

void *request_connection(void *sock_fd)
{

    int new_serv_sockfd = *(int *)sock_fd;

    char msg_buffer[1024];

    while (1)
    {

        memset(msg_buffer, 0, sizeof(msg_buffer));
        int n = read(new_serv_sockfd, msg_buffer, 1024);

        if (n < 0)
        {
            perror("Error in reading..");
            exit(-1);
        }
        cout << "Client: " << msg_buffer;
        vector<string> parseVector;

        string parseInput = string(msg_buffer);
        string dumStr = "";
        /* for(int i=0;i<parseInput.length();i++)
        {
            if(parseInput[i]==' ')
            {
                parseVector.push_back(dumStr);
                dumStr="";
            }
            else
            {
                dumStr+=parseInput[i];
            }
            
        }  */

        int i = 0;
        int j = 0;

        while (i < parseInput.length())
        {
            if (parseInput[i] == ' ' || parseInput[i] == '\n')
            {
                parseVector.push_back(dumStr);
                dumStr = "";
                j = i + 1;
                while (j < parseInput.length() && (parseInput[j] == ' ' || parseInput[j] == '\n'))
                {
                    j++;
                }
                i = j - 1;
            }
            else
            {
                dumStr += parseInput[i];
            }

            i++;
        }
        parseVector.push_back(dumStr);

        string cli_port = parseVector[parseVector.size() - 2];
        string cli_ip = parseVector[parseVector.size() - 1];
        cout << endl;

        // cout<<"Cli-Port "<<cli_port<<" Cli-IP: "<<cli_ip<<endl;

        string checkStringCmd = parseVector[0];
        string invalStr = ""; // Stores the message from server to client

        memset(msg_buffer, 0, sizeof(msg_buffer));

        /*****************Commands Sent from Client side *******************************/

        if (checkStringCmd == "create_user") // create_user command
        {

            if (parseVector.size() - 2 != 3)
            {

                invalStr = "E:Invalid Arguments in create_user command...";
                /*    cout<<"Cli Port "<<cli_port<<" "<<cli_ip<<endl;
                 cout<<parseVector.size()<<endl;
                 for(auto i:parseVector)
                 cout<<i<<endl;
                 cout<<parseVector.size()<<endl;  */
                // strcpy(msg_buffer,invalStr.c_str());
            }
           
            else
            {
                string uid = parseVector[1];
                string pwd = parseVector[2];

                if (uid_check.find(uid) == uid_check.end()) // Valid unique id
                {
                    uid_check.insert(uid);
                    clientDetails[uid] = make_pair(pwd, 0);
                    clientIpPortDetails[uid] = make_pair(cli_ip, cli_port);

                    invalStr = "S:Successfully registered...";
                }
                else if( clientDetails[uid].second ==1)
                {
                        invalStr = "E:User is already logged in..."; 
                }
                else
                {
                    invalStr = "E:User Id already exists...";
                }
            }
        }

        else if (checkStringCmd == "login") // login command
        {
            if (parseVector.size() - 2 != 3)
            {

                invalStr = "E:Invalid number of Arguments in login command...";
                // strcpy(msg_buffer,invalStr.c_str());
            }
            else
            {
                string uid = parseVector[1];
                string pw=parseVector[2];

                if (uid_check.find(uid) == uid_check.end()) // id does not exist
                {

                    invalStr = "E:This id does not exist...";
                }
                else if(clientDetails[uid].second==1)
                {
                    invalStr = "S:User is already logged in";
                }
                else if( clientDetails[uid].first!=pw)  // Incorrect password is provided
                {
                     invalStr = "S:Incorrect Password";
                }
                else
                {
                    clientDetails[uid].second = 1;
                    login_flag = 1;

                    invalStr = "S:successfully logged in for " + uid + " ";
                }
            }
        }
        else if (checkStringCmd == "logout")
        {
            if (parseVector.size() - 2 != 1)
            {

                invalStr = "E:Invalid number of Arguments in logout command...";
                // strcpy(msg_buffer,invalStr.c_str());
            }
            else
            {

                string UserId = getUserId(cli_port);

                if (UserId == "" || uid_check.find(UserId) == uid_check.end()) // id does not exist
                {

                    invalStr = "E:This id does not exist...";
                }
                else if (clientDetails[UserId].second == 1) // Means the user is currently logged in
                {
                    clientDetails[UserId].second = 0;
                   // login_flag = 0; // login_flag is set to 0

                    invalStr = "S:successfully logged out for " + UserId + " ";
                }
                else
                {
                    invalStr = "E:This User id is currently not logged in " + UserId + " ";
                }
            }
        }

        else if (checkStringCmd == "create_group") // create_group command
        {
            string UserId = getUserId(cli_port);

            // unordered_map<string,string> groupAdmin;  // groupId,userId
            //  unordered_map<string,vector<string>> groupMembers;  // groupId,Members of the group

            if (parseVector.size() - 2 != 2)
            {

                invalStr = "E:Invalid number of Arguments in create_group command...";
                // strcpy(msg_buffer,invalStr.c_str());
            }
            else if (clientDetails[UserId].second==0)   // User Id is not logged in
            {
                invalStr = "E:User Id " + UserId + " is not logged in";
            }
            else if (uid_check.find(UserId) == uid_check.end()) // Current user is not currently regitered with the tracker
            {

                invalStr = "E:No such userId is currently registered..";
            }

            else
            {
                string groupId = parseVector[1];

                if (groupMembers.find(groupId) == groupMembers.end()) // No such group Id exists
                {

                    groupAdmin[groupId] = UserId;
                    groupMembers[groupId].insert(UserId);
                    invalStr = "S:Group is successfully created with group admin " + UserId;
                }
                else
                {
                    invalStr = "E:This group currently exists with group admin " + UserId;
                }
            }
        }
        else if (checkStringCmd == "join_group") // join_group command
        {
            string UserId = getUserId(cli_port);

            // unordered_map<string,string> groupAdmin;  // groupId,userId
            //  unordered_map<string,set<string>> groupMembers;  // groupId,Members of the group
            // unordered_map<string,set<string>> pend_request;  //  groupId,Members of the group

            if (parseVector.size() - 2 != 2)
            {

                invalStr = "E:Invalid number of Arguments in join_group command...";
                // strcpy(msg_buffer,invalStr.c_str());
            }
            else if (clientDetails[UserId].second==0) // User Id is not logged in
            {
                invalStr = "E:User Id " + UserId + " is not logged in";
            }
            else if (uid_check.find(UserId) == uid_check.end()) // Current user is not currently regitered with the tracker
            {

                invalStr = "E:No such userId is currently registered..";
            }

            else
            {
                string gid = parseVector[1];

                if (groupMembers.find(gid) == groupMembers.end()) // If no such group id exists
                {
                    invalStr = "E:No such group Id exists.";
                }
                else if (groupMembers[gid].find(UserId) != groupMembers[gid].end()) // If the user is already in the group
                {
                    invalStr = "S:User id:" + UserId + " is already present in the group with grp id: " + gid;
                }
                else
                {
                    string g_admin = groupAdmin[gid];
                    //  groupMembers[gid].insert(UserId);
                    pend_request[gid].insert(UserId);
                    invalStr = "S:User id:" + UserId + " has been successfully added to Pending queue with Group Id:" + gid + " with Group Admin:" + g_admin;
                }
            }
        }

        else if (checkStringCmd == "accept_request") // accept_request command
        {
            string UserId = getUserId(cli_port);

            // unordered_map<string,string> groupAdmin;  // groupId,userId
            //  unordered_map<string,set<string>> groupMembers;  // groupId,Members of the group
            //unordered_map<string,set<string>> pend_request;  //  groupId,Members of the group

            if (parseVector.size() - 2 != 3)
            {

                invalStr = "E:Invalid number of Arguments in accept_request command...";
                // strcpy(msg_buffer,invalStr.c_str());
            }
            else
            {
                string gid = parseVector[1];
                string req_uid = parseVector[2];

                if (clientDetails[UserId].second==0) // User Id is not logged in
                {
                    invalStr = "E:User Id " + UserId + " is not logged in";
                }

                else if (groupAdmin[gid] != UserId) // User Id is not the admin of this group
                {
                    invalStr = "E: " + UserId + " is not the admin of the group: " + gid;
                }
                else if (pend_request[gid].find(req_uid) == pend_request[gid].end()) // If req_uid is not in the pend_request
                {
                    invalStr = "E: Requested User id: " + req_uid + " is not in the Pending list for group: " + gid;
                }
                else
                {

                    groupMembers[gid].insert(req_uid);
                    pend_request[gid].erase(req_uid);
                    invalStr = "S: Requested User id: " + req_uid + " added to group: " + gid+" and successfully removed from the pending queue";
                }
            }
        }

        else if (checkStringCmd == "leave_group") // leave_group command
        {
            string UserId = getUserId(cli_port);

            // unordered_map<string,string> groupAdmin;  // groupId,userId
            //  unordered_map<string,set<string>> groupMembers;  // groupId,Members of the group
            //unordered_map<string,set<string>> pend_request;  //  groupId,Members of the group

            if (parseVector.size() - 2 != 2)
            {

                invalStr = "E:Invalid number of Arguments in leave_group command...";
                // strcpy(msg_buffer,invalStr.c_str());
            }
            else
            {
                string gid = parseVector[1];
                if (clientDetails[UserId].second==0) // User Id is not logged in
                {
                    invalStr = "E:User Id " + UserId + " is not logged in";
                }
                else if (groupMembers.find(gid) == groupMembers.end()) // If the group does not exist
                {
                    invalStr = "E:  Group id: " + gid + " does not exist";
                }
                else if (groupAdmin[gid] == UserId) // If the User Id of the requester is same as that of the user id of the group admin
                {
                    invalStr = "E: Group Admin with Group id: " + gid + " cannot leave the group";
                }

                else if (groupMembers[gid].find(UserId) == groupMembers[gid].end()) // If User Id is not in the group
                {
                    invalStr = "E:  User id: " + UserId + " is not in the group: " + gid;
                }
                else
                {

                    groupMembers[gid].erase(UserId);
                    invalStr = "S: User id: " + UserId + " successfully removed from the group: " + gid;
                }
            }
        }

        else if (checkStringCmd == "list_requests") // list pending join command for group id given as input
        {
            string UserId = getUserId(cli_port);

            // unordered_map<string,string> groupAdmin;  // groupId,userId
            //  unordered_map<string,set<string>> groupMembers;  // groupId,Members of the group
            //unordered_map<string,set<string>> pend_request;  //  groupId,Members of the group

            if (parseVector.size() - 2 != 2)
            {

                invalStr = "E:Invalid number of Arguments in list_requests command...";
                // strcpy(msg_buffer,invalStr.c_str());
            }
            else
            {
                string gid = parseVector[1];

                if (clientDetails[UserId].second==0) // User Id is not logged in
                {
                    invalStr = "E:User Id " + UserId + " is not logged in";
                }

                else if (groupMembers[gid].empty()) // If the set is empty
                {
                    invalStr = "E: No Pending requests are present";
                }
                else
                {
                    set<string> temp = groupMembers[gid];

                    invalStr = "Pending requests are : ";
                    for (auto s : temp)
                        invalStr += s + " ";
                }
            }
        }

        else if (checkStringCmd == "list_groups") // list all available groups in the network
        {
            string UserId = getUserId(cli_port);

            // unordered_map<string,string> groupAdmin;  // groupId,userId
            //  unordered_map<string,set<string>> groupMembers;  // groupId,Members of the group
            //unordered_map<string,set<string>> pend_request;  //  groupId,Members of the group

            if (parseVector.size() - 2 != 1)
            {

                invalStr = "E:Invalid number of Arguments in list_groups command...";
                // strcpy(msg_buffer,invalStr.c_str());
            }
            else
            {
                if (clientDetails[UserId].second==0) // User Id is not logged in
                {
                    invalStr = "E:User Id " + UserId + " is not logged in";
                }

                else if (groupAdmin.empty()) // If the map is empty
                {
                    invalStr = "E: No groups are present";
                }
                else
                {

                    invalStr = "List of groups are : ";
                    string k;
                    for (auto it = groupAdmin.begin(); it != groupAdmin.end(); ++it)
                    {
                        k = it->first;
                        invalStr += k + " ";
                    }
                }
            }
        }

        else if (checkStringCmd == "upload_file") // list all available groups in the network
        {
            string UserId = getUserId(cli_port);

            // unordered_map<string,string> groupAdmin;  // groupId,userId
            //  unordered_map<string,set<string>> groupMembers;  // groupId,Members of the group
            //unordered_map<string,set<string>> pend_request;  //  groupId,Members of the group

            if (parseVector.size() - 4 != 3)
            {

                invalStr = "E:Invalid number of Arguments in upload_file command...";
                // strcpy(msg_buffer,invalStr.c_str());
            }

            else
            {
                string filePath = parseVector[1];
                string groupId = parseVector[2];
                string fileSize = parseVector[3];
                string shaHash = parseVector[4];

                if (clientDetails[UserId].second==0) // User Id is not logged in
                {
                    invalStr = "E:User Id " + UserId + " is not logged in";
                }
                else if (groupMembers.find(groupId) == groupMembers.end()) // If group id does not exist
                {

                    invalStr = "E:Group Id " + groupId + " does not exist";
                }
                else if (groupMembers[groupId].find(UserId) == groupMembers[groupId].end()) // User Id is not present in the group
                {
                    invalStr = "E:User Id " + UserId + " does not exist";
                }
                else if (open(filePath.c_str(), O_RDONLY) == -1)
                {

                    invalStr = "E:File Path does not exist";
                }
                else
                {
                    //  unordered_map<string,vector<vector<string>>> uploadList;  // groupId -> file path, file size, sha hash, port_no
                    /*   struct FILES
                    {
                        string user_id;
                            string file_path;
                            string file_size;
                     string sha_hash;
                    string port_no;

                        }

                vector<struct FILES> fileInfo;
                unordered_map<string,vector<struct FILES>> uploadFileGrpId; */
                    // groupId -> file path, file size, sha hash, port_no//

                    /*  if(fileSize=="0")
                     {
                         invalStr="E:File does not exist...";
                     }
                     else
                     { */
                    struct FILES f_info;

                    f_info.user_id = UserId;
                    f_info.file_path = filePath;
                    f_info.file_size = fileSize;
                    f_info.sha_hash = shaHash;
                    f_info.port_no = cli_port;
                    f_info.file_name = getFileName(filePath);
                    f_info.ip_adr = cli_ip;
                    f_info.no_chunks = getTotalChunks(fileSize);

                    uploadFileGrpId[groupId].push_back(f_info);
                 //   invalStr = "S:File Details have been added to group Id: " + groupId + " " + uploadFileGrpId[groupId][0].file_size;
                 invalStr = "S:File Details have been added to group Id: " + groupId;
                    // }
                }
            }
        }

        else if (checkStringCmd == "download_file") // list all available groups in the network
        {
            string UserId = getUserId(cli_port);

            // download_file​ <group_id> <file_name> <destination_path>

            // unordered_map<string,string> groupAdmin;  // groupId,userId
            //  unordered_map<string,set<string>> groupMembers;  // groupId,Members of the group
            //unordered_map<string,set<string>> pend_request;  //  groupId,Members of the group

            if (parseVector.size() - 2 != 4)
            {

                invalStr = "E:Invalid number of Arguments in download_file command...";
                // strcpy(msg_buffer,invalStr.c_str());
            }

            else
            {
                string groupId = parseVector[1];
                string fileName = parseVector[2];
                string destPath = parseVector[3];

                if (clientDetails[UserId].second==0) // User Id is not logged in
                {
                    invalStr = "E:User Id " + UserId + " is not logged in";
                }
                else if (groupMembers.find(groupId) == groupMembers.end()) // If group id does not exist
                {

                    invalStr = "E:Group Id " + groupId + " does not exist";
                }
                else if (groupMembers[groupId].find(UserId) == groupMembers[groupId].end()) // User Id is not present in the group
                {
                    invalStr = "E:User Id " + UserId + " does not exist in the group";
                }
                else if (uploadFileGrpId[groupId].size() == 0) // If NO files are present in this grp
                {
                    invalStr = "E:No files are present in the group";
                }

                else
                {
                    //  unordered_map<string,vector<vector<string>>> uploadList;  // groupId -> file path, file size, sha hash, port_no
                    /*   struct FILES
                    {
                        string user_id;
                            string file_path;
                            string file_size;
                     string sha_hash;
                    string port_no;

                        }

                vector<struct FILES> fileInfo;
                unordered_map<string,vector<struct FILES>> uploadFileGrpId; */
                    // groupId -> file path, file size, sha hash, port_no//

                    /*  if(fileSize=="0")
                     {
                         invalStr="E:File does not exist...";
                     }
                     else
                     { */
                    /*    struct FILES f_info;

                          f_info.user_id=UserId;
                          f_info.file_path=filePath;
                          f_info.file_size=fileSize;
                          f_info.sha_hash=shaHash;
                          f_info.port_no=cli_port;
                          f_info.file_name=getFileName(filePath);
                          f_info.ip_adr=cli_ip;
                          f_info.no_chunks=getTotalChunks(fileSize);

                          uploadFileGrpId[groupId].push_back(f_info);  */
                    //    invalStr="S:File Details have been added to group Id: "+groupId+" "+uploadFileGrpId[groupId][0].file_size;

                    unsigned long long int i = 0;
                    string evalStr = "";
                    for (i = 0; i < uploadFileGrpId[groupId].size(); i++)
                    {

                        if (uploadFileGrpId[groupId][i].file_name == fileName)
                        {
                            evalStr += uploadFileGrpId[groupId][i].ip_adr + " " + uploadFileGrpId[groupId][i].port_no + " " + uploadFileGrpId[groupId][i].no_chunks + " " + fileName + " " + uploadFileGrpId[groupId][i].file_size + "|";

                            // ip port chunks filename filesize
                        }
                    }

                    if (evalStr == "")
                    {
                        invalStr = "E:File does not exist...";
                    }
                    else
                    {
                        invalStr = "S: " + evalStr; // // ip port chunks filename filesize
                    }

                    // }
                }
            }
        }

        else if (checkStringCmd == "list_files") // list_files​ <group_id>  // List all the files in that group
        {
            string UserId = getUserId(cli_port);

            // unordered_map<string,string> groupAdmin;  // groupId,userId
            //  unordered_map<string,set<string>> groupMembers;  // groupId,Members of the group
            //unordered_map<string,set<string>> pend_request;  //  groupId,Members of the group

            if (parseVector.size() - 2 != 2)
            {

                invalStr = "E:Invalid number of Arguments in list_files command...";
                // strcpy(msg_buffer,invalStr.c_str());
            }

            else
            {

                string groupId = parseVector[1];
                vector<struct FILES> fileContent;
                set<string> file;

                if (clientDetails[UserId].second==0) // User Id is not logged in
                {
                    invalStr = "E:User Id " + UserId + " is not logged in";
                }
                else if (groupMembers.find(groupId) == groupMembers.end()) // If group id does not exist
                {

                    invalStr = "E:Group Id " + groupId + " does not exist";
                }
                else if (groupMembers[groupId].find(UserId) == groupMembers[groupId].end()) // User Id is not present in the group
                {
                    invalStr = "E:User Id " + UserId + " does not exist";
                }

                else
                {
                    //  unordered_map<string,vector<vector<string>>> uploadList;  // groupId -> file path, file size, sha hash, port_no
                    /*   struct FILES
                    {
                        string user_id;
                            string file_path;
                            string file_size;
                     string sha_hash;
                    string port_no;

                        }

                vector<struct FILES> fileInfo;
                unordered_map<string,vector<struct FILES>> uploadFileGrpId; */
                    // groupId -> file path, file size, sha hash, port_no//

                    /*  if(fileSize=="0")
                     {
                         invalStr="E:File does not exist...";
                     }
                     else
                     { */
                    /*   struct FILES f_info;

                          f_info.user_id=UserId;
                          f_info.file_path=filePath;
                          f_info.file_size=fileSize;
                          f_info.sha_hash=shaHash;
                          f_info.port_no=cli_port;
                          f_info.file_name=getFileName(filePath);
                          f_info.ip_adr=cli_ip;
                          f_info.no_chunks=getTotalChunks(fileSize);

                          uploadFileGrpId[groupId].push_back(f_info);  */

                    //vector<string> fileContent;
                    //set<string> file;

                    fileContent = uploadFileGrpId[groupId];

                    invalStr = "S:List of files for group Id: " + groupId + " are: ";

                    for (auto i : fileContent)
                    {
                        file.insert(i.file_name);
                    }
                    for (auto s : file)
                    {
                        invalStr += s + "|";
                    }

                    //   invalStr="S:File Details have been added to group Id: "+groupId+" "+uploadFileGrpId[groupId][0].file_size;
                    // }
                }
            }
        }

        else
        {
            //fgets(msg_buffer,1024,stdin);
            invalStr = "Invalid command...";
        }

        strcpy(msg_buffer, invalStr.c_str());

        n = write(new_serv_sockfd, msg_buffer, strlen(msg_buffer));

        if (n < 0)
        {
            perror("Error in writing..");
            exit(-1);
        }

        /*  memset(msg_buffer,0,sizeof(msg_buffer));

        fgets(msg_buffer,1024,stdin);

        n=write(new_serv_sockfd,msg_buffer,strlen(msg_buffer));

         if(n<0)
         {
           perror("Error in writing..");
           exit(-1);
         }  */

        /*  int i=strncmp("Bye",msg_buffer,3);

         if(i==0)
         break;  */
    }

    return NULL;
}

int main(int argc, char *argv[])
{

    int serv_sockfd;
    int new_serv_sockfd;
    struct sockaddr_in serv_addr;
    int opt = 1;

    serv_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_sockfd < 0)
    {
        perror("Error in creating server socket..");
        exit(-1);
    }

    if (setsockopt(serv_sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY; // Makes it compatible against all available interfaces (0.0.0.0)
    serv_addr.sin_port = htons(8880);

    if (bind(serv_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Binding with Port Number failed....");
        exit(-1);
    }

    if (listen(serv_sockfd, 10) < 0) // 10 is the queue size assigned
    {
        perror("Failed to listen.... ");
        exit(-1);
    }

    socklen_t addr_len = sizeof(serv_addr);

    pthread_t t_id;

    while (1)
    {

        if ((new_serv_sockfd = accept(serv_sockfd, (struct sockaddr *)&serv_addr, (socklen_t *)&addr_len)) < 0)
        {
            perror("Failed to accept...");
            exit(-1);
        }

        if (pthread_create(&t_id, NULL, request_connection, (void *)&new_serv_sockfd) < 0)
        {
            perror("Error in creating thread...");
            exit(-1);
        }

        //pthread_join(t_id , NULL);
    }

    close(new_serv_sockfd);
    close(serv_sockfd);

    return 0;
}
