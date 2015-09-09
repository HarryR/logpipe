# apacheclf2json

[![Build Status](https://drone.io/github.com/HarryR/apacheclf2json/status.png)](https://drone.io/github.com/HarryR/apacheclf2json/latest)

Converts apache common log files into ElasticSearch bulk-import format JSON compatible with Logstash or Kibana.

It was developed to perform bulk-import of HTTP logfiles into ElasticSearch because Logstash was too slow.
