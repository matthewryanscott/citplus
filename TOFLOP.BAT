cd docs
pkzip -ex -f docs
copy docs.zip ..
cd ..
pkzip -ex -f cit-src
copy cit-src.zip a:
fc /b cit-src.zip a:*.*
