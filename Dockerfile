# Copyright (C) 2019 Red Hat, Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

FROM httpd:2.4 as builder

RUN mkdir -p /output/bindep
RUN apt-get update && apt-get install -y python3-pip git && pip3 install bindep
COPY bindep.txt /bindep.txt
RUN cd / \
  && bindep -l newline > /output/bindep/run.txt \
  && apt-get install -y $(bindep -b compile)
COPY . /src
RUN cd /src \
  && autoreconf -fi \
  && ./configure --with-comment=$(git describe --always) \
  && make \
  && make install

FROM httpd:2.4

COPY ./rewrite.conf /usr/local/apache/conf/extra/rewrite.conf
# Turn off all of the modules first, then enable proxy and rewrite
RUN sed -i "s/^LoadModule/#LoadModule/" /usr/local/apache2/conf/httpd.conf \
  && for f in proxy_http proxy rewrite ; do \
       sed -i "s/#LoadModule ${f}_module/LoadModule ${f}_module/" /usr/local/apache2/conf/httpd.conf ;\
     done \
  && echo "Include conf/extra/rewrite.conf" >> /usr/local/apache2/conf/httpd.conf
COPY --from=builder /output/bindep/run.txt /run.txt
COPY --from=builder /usr/local/bin/zuul-preview /usr/local/bin/zuul-preview
RUN apt-get update \
  && apt-get install -y $(cat /run.txt) \
  && apt-get clean \
  && rm -rf /var/lib/apt/lists/* /run.txt
