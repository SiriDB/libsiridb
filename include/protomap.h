/*
 * protomap.h
 *
 *  Created on: Apr 07, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#ifndef SIRIDB_PROTOMAP_H_
#define SIRIDB_PROTOMAP_H_

// CprotoReqQuery for sending queries
#define CprotoReqQuery 0

// CprotoReqInsert for sending inserts
#define CprotoReqInsert 1

// CprotoReqAuth for authentication
#define CprotoReqAuth 2

// CprotoReqPing for ping on the connection
#define CprotoReqPing 3

//CprotoReqAdmin for a manage server request
#define CprotoReqAdmin 32

// CprotoResQuery on query response
#define CprotoResQuery 0

// CprotoResInsert on insert response
#define CprotoResInsert 1

// CprotoResAuthSuccess on authentication success
#define CprotoResAuthSuccess 2

// CprotoResAck on ack
#define CprotoResAck 3

// CprotoResInfo on database info response
#define CprotoResInfo 4

// CprotoResFile on request file response
#define CprotoResFile 5

//CprotoAckAdmin on succesful manage server request
#define CprotoAckAdmin 32

//CprotoAckAdminData on succesful manage server request with data
#define CprotoAckAdminData 33

// CprotoErrMsg general error
#define CprotoErrMsg 64

// CprotoErrQuery on query error
#define CprotoErrQuery 65

// CprotoErrInsert on insert error
#define CprotoErrInsert 66

// CprotoErrServer on server error
#define CprotoErrServer 67

// CprotoErrPool on server error
#define CprotoErrPool 68

// CprotoErrUserAccess on server error
#define CprotoErrUserAccess 69

// CprotoErr on server error
#define CprotoErr 70

// CprotoErrNotAuthenticated on server error
#define CprotoErrNotAuthenticated 71

// CprotoErrAuthCredentials on server error
#define CprotoErrAuthCredentials 72

// CprotoErrAuthUnknownDb on server error
#define CprotoErrAuthUnknownDb 73

// CprotoErrLoadingDb on server error
#define CprotoErrLoadingDb 74

// CprotoErrFile on server error
#define CprotoErrFile 75

// CprotoErrAdmin on manage server error with message
#define CprotoErrAdmin 96

// CprotoErrAdminInvalidRequest on invalid manage server request
#define CprotoErrAdminInvalidRequest 97

// AdminNewAccount for create a new manage server account
#define AdminNewAccount 0

// AdminChangePassword for changing a server account password
#define AdminChangePassword 1

// AdminDropAccount for dropping a server account
#define AdminDropAccount 2

// AdminNewDatabase for creating a new database
#define AdminNewDatabase 3

// AdminNewPool for expanding a database with a new pool
#define AdminNewPool 4

// AdminNewReplica for expanding a database with a new replica
#define AdminNewReplica 5

// AdminGetVersion for getting the siridb server version
#define AdminGetVersion 64

// AdminGetAccounts for getting all accounts on a siridb server
#define AdminGetAccounts 65

// AdminGetDatabases for getting all database running on a siridb server
#define AdminGetDatabases 66

#endif /* SIRIDB_PROTOMAP_H_ */
