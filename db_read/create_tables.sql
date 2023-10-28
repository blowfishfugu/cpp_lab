USE keyword_db

--all known words
CREATE TABLE automation_message_keyword (
    keyword_id BIGINT       NULL,
    word       VARCHAR (30) NULL
);

--contents of a 'sentence'
CREATE TABLE automation_message_keyphrase (
    keyword_id       BIGINT NOT NULL,
    keyword_group_id BIGINT NOT NULL
);

--head of sentence, expected wordcount per topic
--topicname, usermapping and response might be further normalized into additional tables
CREATE TABLE automation_message_keywordgroup (
    keyword_group_id BIGINT       NULL,
    userid           VARCHAR (30) NULL,
    wordcount        BIGINT       NULL,
    topicname        VARCHAR (30) NULL,
    response         VARCHAR (50) NULL
);



