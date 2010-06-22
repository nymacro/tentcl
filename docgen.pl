#!/usr/bin/perl
# Tentcl DocGen
# Aaron Marks 2006-2008
use strict;

my @functions;
my @docs;
my $tentcl_version = get_tentcl_version();

print "<html><head><title>Tentcl DocGen</title></head><body>\n";
print "<h1>Tentcl DocGen ($tentcl_version)</h1>";

while (my $file = shift) {
    open(FILE, "$file");
    my @content = <FILE>;
    close(FILE);

    $file =~ s/\..+$//;

    for (my $i = 0; $i < @content; $i++) {
        if ($content[$i] =~ /^\/\*tcl:/) {
            my ($name, $args) = ($content[$i] =~ /^\/\*tcl: *([^ ]+)(.+)?/);
            chomp $name;
            push @functions, $name;
            my $docstring = "";
            while ($content[++$i] !~ /\*\//) {
                $content[$i] =~ s/ +?\*? *?//;
                if ($content[$i] =~ /^\s*$/) {
                    $docstring = $docstring . '<p />';
                } else {
                    $docstring = $docstring . $content[$i];
                }
            }

            push @docs, "<h3><a name=\"$name\">$name</a></h3>\n".
                "<i>Usage:<b> $name $args</b></i><br />\n".
                "$docstring<p />\n";
        }
    }
}

print "<a name=\"index\"><h2>Index</h2></a>\n";
@functions = sort @functions;
foreach my $i (@functions) {
    print "<a href=\"#$i\">$i</a><br />\n";
}

print "<a name=\"files\"><h2>Functions</h2></a>\n";
@docs = sort @docs;
foreach my $i (@docs) {
    print $i;
    print "<hr />";
}

print '<p /><i>Documentation generated at '.localtime().'</i></body></html>';

sub get_tentcl_version {
    open(FILE, "src/tcl.h");
    my @contents = <FILE>;
    @contents = grep(/^\#define TENTCL_VERSION/, @contents);
    close(FILE);
    $contents[0] =~ /^\#define TENTCL_VERSION (.+)/;
    return $1;
}
