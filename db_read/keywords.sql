SELECT 
count(grp.keyword_group_id) AS hits,
--	words.word, 
	grp.keyword_group_id,
	grp.wordcount AS expected,
	grp.topicname,grp.response 
FROM automation_message_keyword words
	JOIN automation_message_keyphrase phrases 
	ON words.keyword_id=phrases.keyword_id
	JOIN automation_message_keywordgroup grp
	ON grp.keyword_group_id=phrases.keyword_group_id
WHERE grp.userid='altf4' AND words.word 
IN (SELECT * FROM string_split('something about hello world',' '))
GROUP BY grp.keyword_group_id,grp.wordcount,grp.topicname,grp.response
--exact match or sorted possibilities
--HAVING count(grp.keyword_group_id)>=grp.wordcount
ORDER BY count(grp.keyword_group_id) desc