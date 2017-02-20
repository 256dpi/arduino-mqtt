var gulp = require('gulp');
var del = require('del');
var zip = require('gulp-zip');

gulp.task('clean', function(){
  return del(['./build']);
});

gulp.task('copy', ['clean'], function(){
  return gulp.src('./**/*')
    .pipe(gulp.dest('./build'));
});

gulp.task('thin', ['copy'], function(){
  return del([
    './build/update.sh',
    './build/Gulpfile.js',
    './build/package.json',
    './build/node_modules',
    './build/mqtt.zip',
    './build/CMakeLists.txt',
    './build/yun'
  ]);
});

gulp.task('compress', ['thin'], function () {
  return gulp.src('build/**/*')
    .pipe(zip('mqtt.zip'))
    .pipe(gulp.dest('./'));
});

gulp.task('default', ['compress']);
