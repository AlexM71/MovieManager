#include "MemoriesDialog.hpp"
#include "ui_MemoriesDialog.h"

#define cAnimationSpeed 250 // in ms

//#define DEBUG

MemoriesDialog::MemoriesDialog(QString sSavePath, QWidget *parent) : QDialog(parent)
{
    this->m_ui = new Ui::Memories;
    this->setWindowIcon(Common::GetIconAccordingToColorScheme(qApp->styleHints()->colorScheme(), "flashback.png"));
    m_ui->setupUi(this);
    m_movies = new QList<stMovie>;
    m_sSavePath = sSavePath;

    m_displayedIndex = 0;

    GetEphemeris();

    this->setFixedSize(360,480);

    if(m_movies->size() == 0)
    {
        this->done(-1);
    }
    else
    {
        UpdateBackgrounds(eTransition::None);
        this->show();
        this->exec();
    }
}

MemoriesDialog::~MemoriesDialog()
{
    delete m_ui;
    for(int nMovie = 0; nMovie < m_movies->size(); nMovie++)
    {
        delete m_movies->at(nMovie).nYear;
    }
    delete m_movies;
}

void MemoriesDialog::GetEphemeris()
{
    QSqlQuery query;
    if(!query.exec(
            "SELECT movies.ID, Name, Poster, STRFTIME('%Y', ViewDate) AS Year FROM movies INNER JOIN views WHERE movies.ID = views.ID_Movie "
            "AND STRFTIME('%d', ViewDate) = STRFTIME('%d', DATE('now', 'localtime')) " // Select only movies viewed on current day
            "AND STRFTIME('%m', ViewDate) = STRFTIME('%m', DATE('now', 'localtime')) " // Select only movies viewed on current month
            "AND STRFTIME('%Y', ViewDate) < STRFTIME('%Y', DATE('now', 'localtime')) " // Select only movies viewed on previous years
            "GROUP BY Name, Year " // Avoid having the same movie multiple times if seen multiple times the same day
            "ORDER BY Name, Year DESC;"
            ))
    {
        Common::LogDatabaseError(&query);
    }
    while(query.next()) {
        int nMovieIndex = GetMovieIndex(query.value(0).toInt());
        if(nMovieIndex == -1)
        {
            struct stMovie movie;
            movie.nYear = new QList<short>;
            movie.nID = query.value(0).toInt();
            movie.sName = query.value(1).toString();
            movie.sPosterPath = query.value(2).toString();
            movie.nYear->append(query.value(3).toInt());
            movie.display = (enum eDisplay)(rand() % 3);

            m_movies->append(movie);
        }
        else
        {
            m_movies->at(nMovieIndex).nYear->append(query.value(3).toInt());
        }
    }
#ifdef DEBUG
    for(int nMovie = 0; nMovie < m_movies->size(); nMovie++)
    {
        qDebug() << m_movies->at(nMovie).sName << m_movies->at(nMovie).nYear->size();
    }
#endif
}

QPixmap MemoriesDialog::ResizeImage(int* nOffsetX, int* nOffsetY, int nIndex)
{
    QPixmap pixmap;

    if(m_movies->at(nIndex).sPosterPath == "")
        pixmap.load(":/assets/Assets/nocover.png");
    else
        pixmap.load(m_sSavePath + QDir::separator() + "Posters" + QDir::separator() + m_movies->at(nIndex).sPosterPath);

    if((float)pixmap.height()/(float)pixmap.width() > (float)this->height()/(float)this->width())
    {
        // Image verticale
        pixmap = pixmap.scaledToWidth(this->width(), Qt::SmoothTransformation);
        *nOffsetX = 0;
        *nOffsetY = 0 - (pixmap.height() - this->height())/2;
    }
    else
    {
        // Image horizontale
        pixmap = pixmap.scaledToHeight(this->height(), Qt::SmoothTransformation);
        *nOffsetX = 0 - (pixmap.width() - this->width())/2;
        *nOffsetY = 0;
    }

    return pixmap;
}

void MemoriesDialog::AddMovieOverlay(QPixmap* pixmap)
{
    QPainter painter(pixmap);

    QLinearGradient lgrad(QPoint(0, this->height() - 150), QPoint(0, this->height()+500));
    lgrad.setColorAt(0.0, QColor(0,0,0,0));
    lgrad.setColorAt(0.05, QColor(0,0,0,210));
    lgrad.setColorAt(1.0, QColor(0,0,0,210));

    painter.setPen(Qt::transparent);
    painter.setBrush(lgrad);
    painter.drawRect(QRect(0, this->height() - 150, this->width(), this->height()+500));

    painter.setFont(QFont("Arial", 20));
    painter.setPen(Qt::white);

    // Add '...' if title is too long
    QFontMetrics fm(painter.font());
    QString sTitle = fm.elidedText(m_movies->at(m_displayedIndex).sName, Qt::ElideRight, this->width());

    painter.drawText(QRect(0, this->height() - 115, this->width(), this->height()+500), Qt::AlignHCenter, sTitle);

    QString sText = "";
    switch(m_movies->at(m_displayedIndex).display)
    {
    case eDisplay::FirstViewed:
        sText.append(tr("On this day in %1, you saw this movie\n").arg(m_movies->at(m_displayedIndex).nYear->at(m_movies->at(m_displayedIndex).nYear->size()-1)));

        for(int nYear = m_movies->at(m_displayedIndex).nYear->size()-2; nYear >= 0; nYear--)
        {
            if(nYear == m_movies->at(m_displayedIndex).nYear->size()-2)
                sText.append(tr("Since then, you also saw this movie at this date in \n"));
            if(nYear == 0)
            {
                if(m_movies->at(m_displayedIndex).nYear->size() == 2)
                    sText.append(tr("%1").arg(m_movies->at(m_displayedIndex).nYear->at(nYear)));
                else
                {
                    sText.chop(2);
                    sText.append(tr(" and %1.").arg(m_movies->at(m_displayedIndex).nYear->at(nYear)));
                }
            }
            else
                sText.append(tr("%1, ").arg(m_movies->at(m_displayedIndex).nYear->at(nYear)));
        }
        if(m_movies->at(m_displayedIndex).nYear->size() > 3)
            sText.append(tr("\nYou must have a reason to see this movie at this date!"));
        break;
    case eDisplay::LastViewed:
    {
        int n = QDate::currentDate().year() - m_movies->at(m_displayedIndex).nYear->at(0);
        if(n == 1)
            sText.append(tr("%1 year ago, you saw this movie.\n").arg(n));
        else
            sText.append(tr("%1 years ago, you saw this movie.\n").arg(n));

        for(int nYear = 1; nYear < m_movies->at(m_displayedIndex).nYear->size(); nYear++)
        {
            if(nYear == 1)
                sText.append(tr("You also saw this movie on this day in "));


            if(m_movies->at(m_displayedIndex).nYear->size() == 2)
                sText.append(tr("%1").arg(m_movies->at(m_displayedIndex).nYear->at(nYear)));
            else
            {
                if(nYear == m_movies->at(m_displayedIndex).nYear->size()-1)
                {
                    sText.chop(2);
                    sText.append(tr(" and %1.").arg(m_movies->at(m_displayedIndex).nYear->at(nYear)));
                }
                else
                    sText.append(tr("%1, ").arg(m_movies->at(m_displayedIndex).nYear->at(nYear)));
            }
        }

        if(m_movies->at(m_displayedIndex).nYear->size() > 3)
            sText.append(tr("\nYou must have a reason to see this movie at this date!"));
        break;
    }
    case eDisplay::ViewedXTimes:
        if(m_movies->at(m_displayedIndex).nYear->size() == 1)
            sText.append(tr("You saw this movie only once at this date: "));
        else if(m_movies->at(m_displayedIndex).nYear->size() == 2)
                sText.append(tr("You saw this movie twice at this date:\n"));
        else
            sText.append(tr("You saw this movie %1 times at this date:\n").arg(m_movies->at(m_displayedIndex).nYear->size()));

        for(int nYear = m_movies->at(m_displayedIndex).nYear->size()-1; nYear >= 0; nYear--)
        {
            if(m_movies->at(m_displayedIndex).nYear->size() == 1)
                sText.append(tr("%1.").arg(m_movies->at(m_displayedIndex).nYear->at(nYear)));
            else
            {
                if(nYear == 0)
                {
                    sText.chop(2);
                    sText.append(tr(" and %1.").arg(m_movies->at(m_displayedIndex).nYear->at(nYear)));
                }
                else
                    sText.append(tr("%1, ").arg(m_movies->at(m_displayedIndex).nYear->at(nYear)));
            }
        }
        if(m_movies->at(m_displayedIndex).nYear->size() > 3)
            sText.append(tr("\nYou must have a reason to see this movie at this date!"));
        break;
    }


    painter.setFont(QFont("Arial", 10));
    painter.drawText(QRect(10, this->height() - 80, this->width(), this->height()+500), Qt::AlignLeft, sText);

}

void MemoriesDialog::UpdateBackgrounds(enum eTransition eTransition)
{
    if(m_movies->size() == 0)
    {
        Common::Log->append(tr("No movies to be displayed in Memories"), eLog::Warning);
        return;
    }

    if(eTransition == eTransition::None)
    {
        int nOffsetX, nOffsetY;
        QPixmap pixmap = ResizeImage(&nOffsetX, &nOffsetY, m_displayedIndex);


        m_ui->displayedLabel->setGeometry(nOffsetX, nOffsetY, pixmap.width(), pixmap.height());

        AddMovieOverlay(&pixmap);
        m_ui->displayedLabel->setPixmap(pixmap);
    }
    else if(eTransition == eTransition::Previous)
    {
        int nStartX_NEW, nStartY_NEW; // Offsets of the new displayed movie (before the animation)
        int nEndX_NEW, nEndY_NEW;     // Offsets of the new displayed movie (after the animation)

        QPixmap pixmap_OLD = m_ui->displayedLabel->pixmap();
        int nStartX_OLD = m_ui->displayedLabel->geometry().x();
        int nStartY_OLD = m_ui->displayedLabel->geometry().y();

        QPixmap pixmap_NEW = ResizeImage(&nEndX_NEW, &nEndY_NEW, m_displayedIndex);
        nStartX_NEW = nEndX_NEW;
        nStartY_NEW = - pixmap_NEW.height();
        int nEndX_OLD = nStartX_OLD;
        int nEndY_OLD = pixmap_NEW.height() - nEndY_NEW;

#ifdef DEBUG
        qDebug() << "Taille de la fenêtre : " + QString::number(this->width()) + "x" + QString::number(this->height());
        qDebug() << "Taille ancienne image : " + QString::number(pixmap_OLD.width()) + "x" + QString::number(pixmap_OLD.height());
        qDebug() << "Taille nouvelle image : " + QString::number(pixmap_NEW.width()) + "x" + QString::number(pixmap_NEW.height());
        qDebug() << "Coordonnées ancienne (" + QString::number(nStartX_OLD) + ":" + QString::number(nStartY_OLD) + ") ->\t (" + QString::number(nEndX_OLD) + ":" + QString::number(nEndY_OLD) + ")";
        qDebug() << "Coordonnées nouvelle (" + QString::number(nStartX_NEW) + ":" + QString::number(nStartY_NEW) + ") ->\t (" + QString::number(nEndX_NEW) + ":" + QString::number(nEndY_NEW) + ")";
#endif

        AddMovieOverlay(&pixmap_NEW);

        m_ui->secondLabel->setPixmap(pixmap_NEW);
        m_ui->secondLabel->setGeometry(nStartX_NEW, nStartY_NEW, pixmap_NEW.width(), pixmap_NEW.height());

        // Moves the current background out of screen
        QPropertyAnimation *anim_OLD = new QPropertyAnimation(m_ui->displayedLabel, "pos", this);
        anim_OLD->setDuration(cAnimationSpeed);
        anim_OLD->setStartValue(QPoint(nStartX_OLD, nStartY_OLD));
        anim_OLD->setEndValue(QPoint(nEndX_OLD,nEndY_OLD));
        anim_OLD->start();

        // Moves the new background in the screen
        QPropertyAnimation *anim_NEW = new QPropertyAnimation(m_ui->secondLabel, "pos", this);
        anim_NEW->setDuration(cAnimationSpeed);
        anim_NEW->setStartValue(QPoint(nStartX_NEW,nStartY_NEW));
        anim_NEW->setEndValue(QPoint(nEndX_NEW,nEndY_NEW));
        anim_NEW->start();

        QObject::connect(anim_NEW, &QPropertyAnimation::finished, this, [this]() {
            // End of animation, the new poster is set on m_ui->displayedLabel
            UpdateBackgrounds(eTransition::None);
        });
    }
    else if(eTransition == eTransition::Next)
    {
        int nStartX_NEW, nStartY_NEW; // Offsets of the new displayed movie (before the animation)
        int nEndX_NEW, nEndY_NEW;     // Offsets of the new displayed movie (after the animation)

        QPixmap pixmap_OLD = m_ui->displayedLabel->pixmap();
        int nStartX_OLD = m_ui->displayedLabel->geometry().x();
        int nStartY_OLD = m_ui->displayedLabel->geometry().y();

        QPixmap pixmap_NEW = ResizeImage(&nEndX_NEW, &nEndY_NEW, m_displayedIndex);
        nStartX_NEW = nEndX_NEW;
        nStartY_NEW = pixmap_OLD.height() + nStartY_OLD;
        int nEndX_OLD = nStartX_OLD;
        int nEndY_OLD = nEndY_NEW - pixmap_OLD.height();

#ifdef DEBUG
        qDebug() << "Taille de la fenêtre : " + QString::number(this->width()) + "x" + QString::number(this->height());
        qDebug() << "Taille ancienne image : " + QString::number(pixmap_OLD.width()) + "x" + QString::number(pixmap_OLD.height());
        qDebug() << "Taille nouvelle image : " + QString::number(pixmap_NEW.width()) + "x" + QString::number(pixmap_NEW.height());
        qDebug() << "Coordonnées (" + QString::number(nStartX_OLD) + ":" + QString::number(nStartY_OLD) + ") ->\t (" + QString::number(nEndX_OLD) + ":" + QString::number(nEndY_OLD) + ")";
        qDebug() << "Coordonnées (" + QString::number(nStartX_NEW) + ":" + QString::number(nStartY_NEW) + ") ->\t (" + QString::number(nEndX_NEW) + ":" + QString::number(nEndY_NEW) + ")";
#endif

        AddMovieOverlay(&pixmap_NEW);

        m_ui->secondLabel->setPixmap(pixmap_NEW);
        m_ui->secondLabel->setGeometry(nStartX_NEW, nStartY_NEW, pixmap_NEW.width(), pixmap_NEW.height());

        // Moves the new background in the screen
        QPropertyAnimation *anim_OLD = new QPropertyAnimation(m_ui->displayedLabel, "pos", this);
        anim_OLD->setDuration(cAnimationSpeed);
        anim_OLD->setStartValue(QPoint(nStartX_OLD, nStartY_OLD));
        anim_OLD->setEndValue(QPoint(nEndX_OLD,nEndY_OLD));
        anim_OLD->start();

        // Moves the current background out of screen
        QPropertyAnimation *anim_NEW = new QPropertyAnimation(m_ui->secondLabel, "pos", this);
        anim_NEW->setDuration(cAnimationSpeed);
        anim_NEW->setStartValue(QPoint(nStartX_NEW,nStartY_NEW));
        anim_NEW->setEndValue(QPoint(nEndX_NEW,nEndY_NEW));
        anim_NEW->start();

        QObject::connect(anim_NEW, &QPropertyAnimation::finished, this, [this]() {
            // End of animation, the new poster is set on m_ui->displayedLabel
            UpdateBackgrounds(eTransition::None);
        });
    }
}

int MemoriesDialog::GetMovieIndex(short nID)
{
    for(int nMovie = 0; nMovie < m_movies->size(); nMovie++)
    {
        stMovie movie = m_movies->at(nMovie);
        if(m_movies->at(nMovie).nID == nID)
            return nMovie;
    }
    return -1;
}



// Il faut un timeout apres chagement
void MemoriesDialog::wheelEvent(QWheelEvent *event)
{
    // x1.2 to avoid bugs because of unfinished animations
    if(m_Timer.elapsed() < cAnimationSpeed * 1.2 && m_Timer.isValid())
        return;

    if(event->angleDelta().y() > 0)
    {
        // Previous movie
        if(m_displayedIndex > 0)
        {
            m_Timer.restart();
            m_displayedIndex--;
            UpdateBackgrounds(eTransition::Previous);
        }
    }
    else if(event->angleDelta().y() < 0)
    {
        // Next movie
        if(m_displayedIndex < m_movies->size() - 1)
        {
            m_Timer.restart();
            m_displayedIndex++;
            UpdateBackgrounds(eTransition::Next);
        }
    }
}
